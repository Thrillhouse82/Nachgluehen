#include "LivingFreezeEngine.h"

#include <algorithm>
#include <cmath>

namespace nachgluehen
{
void LivingFreezeEngine::prepare(double sampleRate, int blockSize, int numChannels)
{
    currentSampleRate = juce::jmax(1.0, sampleRate);
    maxBlockSize = juce::jmax(1, blockSize);
    ringCapacity = juce::jmax(1, static_cast<int>(currentSampleRate * 4.0));
    recentLeft.assign(static_cast<size_t>(ringCapacity), 0.0f);
    recentRight.assign(static_cast<size_t>(ringCapacity), 0.0f);
    frozenLeft.assign(static_cast<size_t>(ringCapacity), 0.0f);
    frozenRight.assign(static_cast<size_t>(ringCapacity), 0.0f);
    frozenTransient.assign(static_cast<size_t>(ringCapacity), 0.0f);
    maxPositionDriftSamples = currentSampleRate * maxPositionDriftMilliseconds * 0.001;
    driftSmoothingCoefficient = 1.0 - std::exp(-1.0 / (currentSampleRate * 0.08));
    pitchSmoothingCoefficient = 1.0 - std::exp(-1.0 / (currentSampleRate * pitchSmoothingSeconds));
    juce::ignoreUnused(numChannels);
    reset();
}

void LivingFreezeEngine::reset()
{
    std::fill(recentLeft.begin(), recentLeft.end(), 0.0f);
    std::fill(recentRight.begin(), recentRight.end(), 0.0f);
    std::fill(frozenLeft.begin(), frozenLeft.end(), 0.0f);
    std::fill(frozenRight.begin(), frozenRight.end(), 0.0f);
    std::fill(frozenTransient.begin(), frozenTransient.end(), 0.0f);
    ringWrite = recentSamples = capturedLength = 0;
    transitionRemaining = 0;
    wasFrozen = false;
    freezeGain = 0.0f;
    textureGainCompensation = static_cast<float>(textureGainFloor);
    smoothValue = smoothCurrent = 0.0f;
    transientEnvelope = {};
    transientGain = { { 1.0f, 1.0f } };
    dryWetCurrent = 0.5f;
    driftCurrent = driftTarget = 0.0f;
    safetyDriftValue = 0.0f;
    driftUpdateCountdown = driftUpdateSamples;
    for (auto& voice : voices)
        voice = {};
}

void LivingFreezeEngine::setSeed(std::uint32_t seed) noexcept
{
    randomState = seed == 0 ? 0x4e616368u : seed;
}

float LivingFreezeEngine::nextRandom() noexcept
{
    randomState ^= randomState << 13;
    randomState ^= randomState >> 17;
    randomState ^= randomState << 5;
    return static_cast<float>((randomState & 0x00ffffffu) / 16777215.0) * 2.0f - 1.0f;
}

float LivingFreezeEngine::readLinear(const std::vector<float>& source, int length, double position) const noexcept
{
    if (length <= 0)
        return 0.0f;
    while (position < 0.0)
        position += length;
    while (position >= length)
        position -= length;
    const auto first = static_cast<int>(position);
    const auto second = (first + 1) % length;
    const auto fraction = static_cast<float>(position - first);
    return source[static_cast<size_t>(first)] + fraction * (source[static_cast<size_t>(second)] - source[static_cast<size_t>(first)]);
}

void LivingFreezeEngine::captureRecent()
{
    const auto captureSamples = juce::jmax(1, static_cast<int>(currentSampleRate * captureDurationSeconds));
    capturedLength = juce::jmin(recentSamples, captureSamples);
    if (capturedLength <= 0)
        return;
    const auto start = (ringWrite - capturedLength + ringCapacity) % ringCapacity;
    for (int i = 0; i < capturedLength; ++i)
    {
        const auto sourceIndex = (start + i) % ringCapacity;
        frozenLeft[static_cast<size_t>(i)] = recentLeft[static_cast<size_t>(sourceIndex)];
        frozenRight[static_cast<size_t>(i)] = recentRight[static_cast<size_t>(sourceIndex)];
    }
    // Preserve a short, decaying marker for attack-rich capture regions. This
    // lets Smooth reduce the musical source transient before overlapping voices
    // sum it, instead of trying to infer it from the already-dense wet mix.
    auto previousLeft = 0.0f;
    auto previousRight = 0.0f;
    auto previousMagnitude = 0.0f;
    auto slowMagnitude = 0.0f;
    auto transientHold = 0.0f;
    const auto transientDecay = static_cast<float>(std::exp(-1.0 / (currentSampleRate * 0.090)));
    for (int i = 0; i < capturedLength; ++i)
    {
        const auto left = frozenLeft[static_cast<size_t>(i)];
        const auto right = frozenRight[static_cast<size_t>(i)];
        const auto change = juce::jmax(std::abs(left - previousLeft), std::abs(right - previousRight));
        const auto magnitude = juce::jmax(std::abs(left), std::abs(right));
        slowMagnitude += (magnitude - slowMagnitude) * 0.0005f;
        const auto relativeRise = juce::jmax(0.0f, magnitude - previousMagnitude) / (0.01f + slowMagnitude);
        const auto slopeOnset = juce::jlimit(0.0f, 1.0f, (relativeRise - 0.08f) * 4.0f);
        const auto spikeOnset = juce::jlimit(0.0f, 1.0f, (change - 0.008f) * 20.0f);
        const auto detected = juce::jmax(slopeOnset, spikeOnset);
        transientHold = juce::jmax(detected, transientHold * transientDecay);
        frozenTransient[static_cast<size_t>(i)] = transientHold;
        previousLeft = left;
        previousRight = right;
        previousMagnitude = magnitude;
    }
    for (int i = 0; i < textureVoiceCount; ++i)
        initializeVoice(voices[static_cast<size_t>(i)], i);
    safetyDriftValue = driftValue;
}

float LivingFreezeEngine::windowValue(double phase, float smooth) noexcept
{
    phase = juce::jlimit(0.0, 1.0, phase);
    const auto sine = juce::jmax(0.0f, static_cast<float>(std::sin(juce::MathConstants<double>::pi * phase)));
    const auto shape = 1.0f + 2.0f * juce::jlimit(0.0f, 1.0f, smooth);
    return std::pow(sine, shape);
}

float LivingFreezeEngine::pitchDriftAmount(float normalizedDrift) noexcept
{
    const auto clampedDrift = juce::jlimit(0.0f, 1.0f, normalizedDrift);
    return std::pow(clampedDrift, 2.5f);
}

float LivingFreezeEngine::getVoicePlaybackSpeed(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, textureVoiceCount)
        ? static_cast<float>(voices[static_cast<size_t>(index)].playbackSpeed) : 1.0f;
}

float LivingFreezeEngine::getVoiceSpeedTarget(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, textureVoiceCount)
        ? static_cast<float>(voices[static_cast<size_t>(index)].speedTarget) : 1.0f;
}

float LivingFreezeEngine::getVoicePosition(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, textureVoiceCount)
        ? static_cast<float>(voices[static_cast<size_t>(index)].readPosition
                             + voices[static_cast<size_t>(index)].positionOffset) : 0.0f;
}

void LivingFreezeEngine::updateVoiceSafety(TextureVoice& voice) noexcept
{
    const auto length = static_cast<double>(juce::jmax(1, capturedLength));
    const auto maxReadable = juce::jmax(0.0, length - 1.0);
    // Keep the physical read region conservative and stable. Drift only changes
    // offset targets within this region, so a parameter change cannot clamp an
    // audible read head to a new position.
    const auto offsetReserve = 2.0 + maxPositionDriftSamples * 1.15;
    voice.safeReadMin = juce::jmin(offsetReserve, maxReadable);
    voice.safeReadMax = juce::jmax(voice.safeReadMin, maxReadable - offsetReserve);

    const auto maximumAdvance = voice.windowLength * (1.0 + maxPlaybackDrift);
    voice.safeStartMin = voice.safeReadMin;
    voice.safeStartMax = juce::jmax(voice.safeStartMin,
                                    voice.safeReadMax - maximumAdvance);
}

void LivingFreezeEngine::constrainVoiceTargets(TextureVoice& voice) noexcept
{
    const auto stereoReserve = std::abs(voice.stereoTarget) * maxPositionDriftSamples;
    const auto minimumOffset = voice.safeReadMin + stereoReserve - voice.readPosition;
    const auto maximumOffset = voice.safeReadMax - stereoReserve - voice.readPosition;
    voice.positionTarget = juce::jlimit(minimumOffset, maximumOffset, voice.positionTarget);
}

void LivingFreezeEngine::initializeVoice(TextureVoice& voice, int index) noexcept
{
    const auto spacing = static_cast<double>(capturedLength) * voiceSpacingFraction;
    voice.cycle = 0;
    voice.startPosition = std::fmod(static_cast<double>(index) * spacing, static_cast<double>(juce::jmax(1, capturedLength)));
    const auto voiceWindowVariation = 0.46 + 0.04 * static_cast<double>(index % 4) / 3.0;
    voice.windowLength = juce::jmax(4.0, static_cast<double>(capturedLength) * voiceWindowVariation);
    voice.readPosition = voice.startPosition;
    voice.playbackSpeed = 1.0;
    voice.windowPhase = 0.0;
    voice.positionOffset = 0.0;
    voice.positionTarget = 0.0;
    voice.speedTarget = 1.0;
    voice.stereoOffset = 0.0;
    voice.stereoTarget = 0.0;
    updateVoiceSafety(voice);
    voice.startPosition = juce::jlimit(voice.safeStartMin, voice.safeStartMax, voice.startPosition);
    voice.readPosition = voice.startPosition;
    voice.active = capturedLength > 0;
}

void LivingFreezeEngine::restartVoice(TextureVoice& voice, int index) noexcept
{
    ++voice.cycle;
    const auto length = static_cast<double>(juce::jmax(1, capturedLength));
    const auto driftScale = static_cast<double>(driftCurrent) * 0.08;
    const auto randomWindowOffset = driftValue > 0.0f ? nextRandom() : 0.0f;
    const auto voiceWindowVariation = 0.46 + 0.04 * static_cast<double>(index % 4) / 3.0;
    voice.windowLength = juce::jlimit(length * minimumWindowFraction,
                                      length * maximumWindowFraction,
                                      length * (voiceWindowVariation + driftScale * randomWindowOffset));
    updateVoiceSafety(voice);
    const auto safeRange = juce::jmax(0.0, voice.safeStartMax - voice.safeStartMin);
    const auto spacing = juce::jmax(1.0, safeRange * voiceSpacingFraction);
    const auto cycleOffset = safeRange * cycleStartStepFraction * static_cast<double>(voice.cycle);
    const auto candidate = voice.safeStartMin + static_cast<double>(index) * spacing + cycleOffset;
    voice.startPosition = juce::jlimit(voice.safeStartMin, voice.safeStartMax,
                                       std::fmod(candidate - voice.safeStartMin,
                                                 juce::jmax(1.0, safeRange)) + voice.safeStartMin);
    voice.readPosition = voice.startPosition;
    voice.windowPhase = 0.0;
    voice.positionOffset = 0.0;
    voice.positionTarget = 0.0;
    voice.stereoOffset = 0.0;
    voice.stereoTarget = 0.0;
    voice.active = capturedLength > 0;
}

void LivingFreezeEngine::updateVoiceTargets() noexcept
{
    driftTarget = driftValue;
    const auto positionDriftAmount = static_cast<double>(driftValue);
    const auto stereoDriftAmount = static_cast<double>(driftValue);
    const auto pitchAmount = static_cast<double>(pitchDriftAmount(driftValue));
    if (driftValue <= 0.0f)
    {
        for (auto& voice : voices)
        {
            voice.positionTarget = 0.0;
            voice.speedTarget = 1.0;
            voice.stereoTarget = 0.0;
        }
        return;
    }

    for (auto& voice : voices)
    {
        voice.positionTarget = nextRandom() * maxPositionDriftSamples * positionDriftAmount;
        const auto step = nextRandom() * pitchRandomWalkStep * pitchAmount;
        voice.speedTarget = juce::jlimit(1.0 - maxPlaybackDrift,
                                         1.0 + maxPlaybackDrift,
                                         voice.speedTarget + step);
        voice.stereoTarget = nextRandom() * 0.15 * stereoDriftAmount;

        constrainVoiceTargets(voice);
    }
}

float LivingFreezeEngine::renderTexture(int channel) noexcept
{
    if (capturedLength <= 0)
        return 0.0f;

    const auto& source = channel == 0 ? frozenLeft : frozenRight;
    double output = 0.0;
    double envelopeEnergy = 0.0;
    for (auto& voice : voices)
    {
        if (!voice.active)
            continue;

        const auto envelope = static_cast<double>(windowValue(voice.windowPhase, smoothCurrent));
        const auto stereoPosition = channel == 0 ? -voice.stereoOffset : voice.stereoOffset;
        const auto rawPosition = voice.readPosition + voice.positionOffset + stereoPosition * maxPositionDriftSamples;
        const auto position = juce::jlimit(voice.safeReadMin, voice.safeReadMax, rawPosition);
        const auto capturedTransient = readLinear(frozenTransient, capturedLength, position);
        const auto sourceGain = 1.0f - 0.98f * smoothCurrent * capturedTransient;
        output += static_cast<double>(readLinear(source, capturedLength, position) * sourceGain) * envelope;
        envelopeEnergy += envelope * envelope;
    }
    if (channel == 0)
    {
        const auto energyCompensation = textureEnergyVoiceScale
            / std::sqrt(juce::jmax(1.0, envelopeEnergy));
        const auto correlationCompensation = 1.0 / juce::jmax(1.0, std::sqrt(envelopeEnergy) * 2.0);
        const auto targetCompensation = juce::jlimit(textureGainFloor, textureGainCeiling,
            0.5 * (energyCompensation + correlationCompensation));
        const auto gainSmoothingSeconds = textureGainSmoothingSeconds
            + 0.24 * static_cast<double>(smoothCurrent * smoothCurrent);
        const auto smoothing = 1.0 - std::exp(-1.0 / (currentSampleRate * gainSmoothingSeconds));
        textureGainCompensation += static_cast<float>((targetCompensation - textureGainCompensation) * smoothing);
    }
    auto wetSample = static_cast<float>(output * static_cast<double>(textureGainCompensation));
    const auto channelIndex = channel == 0 ? 0 : 1;
    const auto magnitude = std::abs(wetSample);
    const auto envelopeCoefficient = magnitude > transientEnvelope[static_cast<size_t>(channelIndex)]
        ? 0.02f : 0.0003f;
    transientEnvelope[static_cast<size_t>(channelIndex)] += (magnitude - transientEnvelope[static_cast<size_t>(channelIndex)]) * envelopeCoefficient;
    const auto attack = juce::jmax(0.0f, magnitude - transientEnvelope[static_cast<size_t>(channelIndex)]);
    const auto targetTransientGain = 1.0f / (1.0f + 8.0f * smoothCurrent * attack);
    const auto gainCoefficient = targetTransientGain < transientGain[static_cast<size_t>(channelIndex)] ? 0.40f : 0.001f;
    transientGain[static_cast<size_t>(channelIndex)] += (targetTransientGain - transientGain[static_cast<size_t>(channelIndex)]) * gainCoefficient;
    return wetSample * transientGain[static_cast<size_t>(channelIndex)];
}

void LivingFreezeEngine::process(juce::AudioBuffer<float>& buffer, bool freeze, float drift, float dryWet, float smooth)
{
    const auto numSamples = buffer.getNumSamples();
    const auto channels = juce::jmin(2, buffer.getNumChannels());
    driftValue = juce::jlimit(0.0f, 1.0f, drift);
    dryWetValue = juce::jlimit(0.0f, 1.0f, dryWet);
    smoothValue = juce::jlimit(0.0f, 1.0f, smooth);

    if (freeze && capturedLength > 0 && driftValue != safetyDriftValue)
    {
        for (auto& voice : voices)
        {
            updateVoiceSafety(voice);
            constrainVoiceTargets(voice);
        }
        safetyDriftValue = driftValue;
        updateVoiceTargets();
    }

    if (freeze && !wasFrozen)
    {
        captureRecent();
        if (driftValue > 0.0f)
            updateVoiceTargets();
        transitionRemaining = transitionSamples;
    }
    else if (!freeze && wasFrozen)
    {
        transitionRemaining = transitionSamples;
    }
    wasFrozen = freeze;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto left = channels > 0 ? buffer.getSample(0, sample) : 0.0f;
        const auto right = channels > 1 ? buffer.getSample(1, sample) : left;
        if (!freeze)
        {
            recentLeft[static_cast<size_t>(ringWrite)] = left;
            recentRight[static_cast<size_t>(ringWrite)] = right;
            ringWrite = (ringWrite + 1) % ringCapacity;
            recentSamples = juce::jmin(recentSamples + 1, ringCapacity);
        }

        const auto targetGain = freeze && capturedLength > 0 ? 1.0f : 0.0f;
        if (transitionRemaining > 0)
        {
            const auto step = 1.0f / static_cast<float>(transitionSamples);
            freezeGain += (targetGain > freezeGain ? step : -step);
            --transitionRemaining;
        }
        else
        {
            freezeGain = targetGain;
        }
        freezeGain = juce::jlimit(0.0f, 1.0f, freezeGain);
        dryWetCurrent += (dryWetValue - dryWetCurrent) / static_cast<float>(transitionSamples);
        dryWetCurrent = juce::jlimit(0.0f, 1.0f, dryWetCurrent);
        const auto smoothCoefficient = 1.0f - std::exp(-1.0f / static_cast<float>(currentSampleRate * 0.05));
        smoothCurrent += (smoothValue - smoothCurrent) * smoothCoefficient;

        if (driftValue > 0.0f && --driftUpdateCountdown <= 0)
        {
            updateVoiceTargets();
            driftUpdateCountdown = static_cast<int>(currentSampleRate * 0.35)
                + static_cast<int>((nextRandom() + 1.0f) * currentSampleRate * 0.325);
        }
        const auto driftStep = driftValue > 0.0f ? static_cast<float>(driftSmoothingCoefficient) : 1.0f;
        driftCurrent += (driftTarget - driftCurrent) * driftStep;
        if (driftValue == 0.0f)
            driftCurrent = driftTarget = 0.0f;

        const auto wetLeft = renderTexture(0);
        const auto wetRight = renderTexture(1);
        const auto wet = dryWetCurrent * freezeGain;
        const auto outputLeft = left * (1.0f - wet) + wetLeft * wet;
        const auto outputRight = right * (1.0f - wet) + wetRight * wet;
        if (channels > 0)
            buffer.setSample(0, sample, std::isfinite(outputLeft) ? outputLeft : 0.0f);
        if (channels > 1)
            buffer.setSample(1, sample, std::isfinite(outputRight) ? outputRight : 0.0f);

        if (freeze && capturedLength > 0)
        {
            for (int voiceIndex = 0; voiceIndex < textureVoiceCount; ++voiceIndex)
            {
                auto& voice = voices[static_cast<size_t>(voiceIndex)];
                if (!voice.active)
                    continue;
                const auto movementSmoothing = static_cast<double>(driftSmoothingCoefficient);
                voice.positionOffset += (voice.positionTarget - voice.positionOffset) * movementSmoothing;
                voice.playbackSpeed += (voice.speedTarget - voice.playbackSpeed) * pitchSmoothingCoefficient;
                voice.stereoOffset += (voice.stereoTarget - voice.stereoOffset) * movementSmoothing;
                voice.playbackSpeed = juce::jlimit(1.0 - maxPlaybackDrift,
                                                   1.0 + maxPlaybackDrift,
                                                   voice.playbackSpeed);
                // The target is constrained immediately, but the audible offset
                // itself only moves through the normal smoothing response.
                const auto stereoReserve = std::abs(voice.stereoOffset) * maxPositionDriftSamples;
                const auto minimumOffset = voice.safeReadMin + stereoReserve - voice.readPosition;
                const auto maximumOffset = voice.safeReadMax - stereoReserve - voice.readPosition;
                voice.positionOffset = juce::jlimit(minimumOffset, maximumOffset, voice.positionOffset);
                voice.readPosition += voice.playbackSpeed;
                voice.readPosition = juce::jlimit(voice.safeStartMin,
                                                  voice.safeReadMax,
                                                  voice.readPosition);
                voice.windowPhase += voice.playbackSpeed / voice.windowLength;
                if (voice.windowPhase >= 1.0)
                    restartVoice(voice, voiceIndex);
            }
        }
    }
}
}
