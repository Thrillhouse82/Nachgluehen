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
    maxPositionDriftSamples = currentSampleRate * maxPositionDriftMilliseconds * 0.001;
    driftSmoothingCoefficient = 1.0 - std::exp(-1.0 / (currentSampleRate * 0.08));
    movementSmoothingCoefficient = 1.0 - std::exp(-1.0 / (currentSampleRate * movementSmoothingSeconds));
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
    ringWrite = recentSamples = capturedLength = 0;
    transitionRemaining = 0;
    wasFrozen = false;
    freezeGain = 0.0f;
    textureGainCompensation = static_cast<float>(textureGainFloor);
    dryWetCurrent = 0.5f;
    driftCurrent = driftTarget = 0.0f;
    globalPlaybackSpeed = globalPlaybackSpeedTarget = 1.0;
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
    for (int i = 0; i < textureVoiceCount; ++i)
        initializeVoice(voices[static_cast<size_t>(i)], i);
    safetyDriftValue = driftValue;
}

float LivingFreezeEngine::windowValue(double phase) noexcept
{
    phase = juce::jlimit(0.0, 1.0, phase);
    return juce::jmax(0.0f, static_cast<float>(std::sin(juce::MathConstants<double>::pi * phase)));
}

float LivingFreezeEngine::pitchDriftAmount(float normalizedDrift) noexcept
{
    const auto clampedDrift = juce::jlimit(0.0f, 1.0f, normalizedDrift);
    return std::pow(clampedDrift, 2.0f);
}

float LivingFreezeEngine::getVoicePlaybackSpeed(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, textureVoiceCount)
        ? static_cast<float>(voices[static_cast<size_t>(index)].playbackSpeed) : 1.0f;
}

float LivingFreezeEngine::getVoiceSpeedTarget(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, textureVoiceCount)
        ? static_cast<float>(globalPlaybackSpeedTarget * voices[static_cast<size_t>(index)].localPitchTarget) : 1.0f;
}

float LivingFreezeEngine::getVoicePitchFactor(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, textureVoiceCount)
        ? static_cast<float>(voices[static_cast<size_t>(index)].localPitchFactor) : 1.0f;
}

float LivingFreezeEngine::getVoicePosition(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, textureVoiceCount)
        ? static_cast<float>(voices[static_cast<size_t>(index)].readPosition
                             + voices[static_cast<size_t>(index)].positionOffset) : 0.0f;
}

float LivingFreezeEngine::getVoicePositionOffset(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, textureVoiceCount)
        ? static_cast<float>(voices[static_cast<size_t>(index)].positionOffset) : 0.0f;
}

float LivingFreezeEngine::getVoicePositionTarget(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, textureVoiceCount)
        ? static_cast<float>(voices[static_cast<size_t>(index)].positionTarget) : 0.0f;
}

float LivingFreezeEngine::getVoiceStereoOffset(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, textureVoiceCount)
        ? static_cast<float>(voices[static_cast<size_t>(index)].stereoOffset) : 0.0f;
}

float LivingFreezeEngine::getVoiceStereoTarget(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, textureVoiceCount)
        ? static_cast<float>(voices[static_cast<size_t>(index)].stereoTarget) : 0.0f;
}

float LivingFreezeEngine::getVoiceSafeReadMin(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, textureVoiceCount)
        ? static_cast<float>(voices[static_cast<size_t>(index)].safeReadMin) : 0.0f;
}

float LivingFreezeEngine::getVoiceSafeReadMax(int index) const noexcept
{
    return juce::isPositiveAndBelow(index, textureVoiceCount)
        ? static_cast<float>(voices[static_cast<size_t>(index)].safeReadMax) : 0.0f;
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
    voice.localPitchFactor = 1.0;
    voice.localPitchTarget = 1.0;
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
    // Low Drift is primarily a barely perceptible texture movement. Keep its
    // position and stereo targets much smaller than a linear mapping, then
    // retain the full range as the control approaches its maximum.
    const auto movementAmount = std::pow(static_cast<double>(driftValue), 1.35);
    const auto positionDriftAmount = movementAmount;
    const auto stereoDriftAmount = movementAmount;
    const auto pitchAmount = static_cast<double>(pitchDriftAmount(driftValue));
    if (driftValue <= 0.0f)
    {
        globalPlaybackSpeedTarget = 1.0;
        for (auto& voice : voices)
        {
            voice.positionTarget = 0.0;
            voice.localPitchTarget = 1.0;
            voice.stereoTarget = 0.0;
        }
        return;
    }

    const auto globalStep = nextRandom() * globalPitchRandomWalkStep * pitchAmount;
    globalPlaybackSpeedTarget = juce::jlimit(1.0 - maximumGlobalPitchDeviation,
                                              1.0 + maximumGlobalPitchDeviation,
                                              globalPlaybackSpeedTarget + globalStep);
    for (auto& voice : voices)
    {
        voice.positionTarget = nextRandom() * maxPositionDriftSamples * positionDriftAmount;
        const auto localStep = nextRandom() * localPitchRandomWalkStep * pitchAmount;
        voice.localPitchTarget = juce::jlimit(1.0 - localPitchMaximumDeviation,
                                               1.0 + localPitchMaximumDeviation,
                                               voice.localPitchTarget + localStep);
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

        const auto envelope = static_cast<double>(windowValue(voice.windowPhase));
        const auto stereoPosition = channel == 0 ? -voice.stereoOffset : voice.stereoOffset;
        const auto rawPosition = voice.readPosition + voice.positionOffset + stereoPosition * maxPositionDriftSamples;
        const auto position = juce::jlimit(voice.safeReadMin, voice.safeReadMax, rawPosition);
        output += static_cast<double>(readLinear(source, capturedLength, position)) * envelope;
        envelopeEnergy += envelope * envelope;
    }
    if (channel == 0)
    {
        const auto energyCompensation = textureEnergyVoiceScale
            / std::sqrt(juce::jmax(1.0, envelopeEnergy));
        const auto correlationCompensation = 1.0 / juce::jmax(1.0, std::sqrt(envelopeEnergy) * 2.8);
        const auto targetCompensation = juce::jlimit(textureGainFloor, textureGainCeiling,
            0.5 * (energyCompensation + correlationCompensation));
        const auto smoothing = 1.0 - std::exp(-1.0 / (currentSampleRate * textureGainSmoothingSeconds));
        textureGainCompensation += static_cast<float>((targetCompensation - textureGainCompensation) * smoothing);
    }
    return static_cast<float>(output * static_cast<double>(textureGainCompensation));
}

void LivingFreezeEngine::process(juce::AudioBuffer<float>& buffer, bool freeze, float drift, float dryWet)
{
    const auto numSamples = buffer.getNumSamples();
    const auto channels = juce::jmin(2, buffer.getNumChannels());
    driftValue = juce::jlimit(0.0f, 1.0f, drift);
    dryWetValue = juce::jlimit(0.0f, 1.0f, dryWet);

    if (freeze && capturedLength > 0 && driftValue != safetyDriftValue)
    {
        for (auto& voice : voices)
        {
            updateVoiceSafety(voice);
            constrainVoiceTargets(voice);
        }
        safetyDriftValue = driftValue;
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
        if (driftValue > 0.0f && --driftUpdateCountdown <= 0)
        {
            updateVoiceTargets();
            driftUpdateCountdown = static_cast<int>(currentSampleRate * 1.5)
                + static_cast<int>((nextRandom() + 1.0f) * currentSampleRate * 0.75);
        }
        const auto driftStep = driftValue > 0.0f ? static_cast<float>(driftSmoothingCoefficient) : 1.0f;
        driftCurrent += (driftTarget - driftCurrent) * driftStep;
        if (driftValue == 0.0f)
        {
            driftCurrent = driftTarget = 0.0f;
            globalPlaybackSpeed = globalPlaybackSpeedTarget = 1.0;
            for (auto& voice : voices)
            {
                voice.localPitchFactor = 1.0;
                voice.localPitchTarget = 1.0;
                voice.playbackSpeed = 1.0;
            }
        }
        else
        {
            globalPlaybackSpeed += (globalPlaybackSpeedTarget - globalPlaybackSpeed) * pitchSmoothingCoefficient;
        }

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
                const auto movementSmoothing = movementSmoothingCoefficient;
                voice.positionOffset += (voice.positionTarget - voice.positionOffset) * movementSmoothing;
                voice.localPitchFactor += (voice.localPitchTarget - voice.localPitchFactor) * pitchSmoothingCoefficient;
                voice.stereoOffset += (voice.stereoTarget - voice.stereoOffset) * movementSmoothing;
                voice.localPitchFactor = juce::jlimit(1.0 - localPitchMaximumDeviation,
                                                       1.0 + localPitchMaximumDeviation,
                                                       voice.localPitchFactor);
                voice.playbackSpeed = juce::jlimit(1.0 - maxPlaybackDrift,
                                                   1.0 + maxPlaybackDrift,
                                                   globalPlaybackSpeed * voice.localPitchFactor);
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
