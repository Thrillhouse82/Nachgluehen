#include "LivingFreezeEngine.h"

#include <algorithm>
#include <cmath>

namespace nachgluehen
{
void LivingFreezeEngine::prepare(double sampleRate, int blockSize, int numChannels)
{
    currentSampleRate = juce::jmax(1.0, sampleRate);
    maxBlockSize = juce::jmax(1, blockSize);
    ringCapacity = juce::jmax(1, static_cast<int>(currentSampleRate * maxCaptureSeconds));
    recentLeft.assign(static_cast<size_t>(ringCapacity), 0.0f);
    recentRight.assign(static_cast<size_t>(ringCapacity), 0.0f);
    frozenLeft.assign(static_cast<size_t>(ringCapacity), 0.0f);
    frozenRight.assign(static_cast<size_t>(ringCapacity), 0.0f);
    crossfadeLength = juce::jlimit(16, 256, static_cast<int>(currentSampleRate * 0.008));
    juce::ignoreUnused(numChannels);
    reset();
}

void LivingFreezeEngine::reset()
{
    std::fill(recentLeft.begin(), recentLeft.end(), 0.0f);
    std::fill(recentRight.begin(), recentRight.end(), 0.0f);
    std::fill(frozenLeft.begin(), frozenLeft.end(), 0.0f);
    std::fill(frozenRight.begin(), frozenRight.end(), 0.0f);
    ringWrite = recentSamples = capturedLength = playbackPosition = 0;
    transitionRemaining = 0;
    wasFrozen = false;
    freezeGain = 0.0f;
    dryWetCurrent = 0.5f;
    driftCurrent = driftTarget = 0.0f;
    driftUpdateCountdown = driftUpdateSamples;
    driftPhase = 0.0;
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
    capturedLength = juce::jmin(recentSamples, ringCapacity);
    if (capturedLength <= 0)
        return;
    const auto start = (ringWrite - capturedLength + ringCapacity) % ringCapacity;
    for (int i = 0; i < capturedLength; ++i)
    {
        const auto sourceIndex = (start + i) % ringCapacity;
        frozenLeft[static_cast<size_t>(i)] = recentLeft[static_cast<size_t>(sourceIndex)];
        frozenRight[static_cast<size_t>(i)] = recentRight[static_cast<size_t>(sourceIndex)];
    }
    playbackPosition = 0.0;
    driftPhase = 0.0;
}

float LivingFreezeEngine::frozenSample(int channel) noexcept
{
    if (capturedLength <= 0)
        return 0.0f;

    const auto channelOffset = channel == 0 ? -1.0 : 1.0;
    const auto position = playbackPosition + channelOffset * static_cast<double>(driftCurrent) * 3.0;
    const auto& source = channel == 0 ? frozenLeft : frozenRight;
    const auto main = readLinear(source, capturedLength, position);
    const auto boundary = juce::jmin(juce::jmax(2, static_cast<int>(crossfadeLength * (1.0f + 0.5f * std::abs(driftCurrent)))), capturedLength / 2);
    if (boundary > 1 && playbackPosition >= capturedLength - boundary)
    {
        const auto progress = static_cast<float>(playbackPosition - (capturedLength - boundary)) / static_cast<float>(boundary);
        const auto beginning = readLinear(source, capturedLength, progress * boundary);
        return main * (1.0f - progress) + beginning * progress;
    }
    return main;
}

void LivingFreezeEngine::process(juce::AudioBuffer<float>& buffer, bool freeze, float drift, float dryWet)
{
    const auto numSamples = buffer.getNumSamples();
    const auto channels = juce::jmin(2, buffer.getNumChannels());
    driftValue = juce::jlimit(0.0f, 1.0f, drift);
    dryWetValue = juce::jlimit(0.0f, 1.0f, dryWet);

    if (freeze && !wasFrozen)
    {
        captureRecent();
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
            driftTarget = nextRandom() * driftValue;
            driftUpdateCountdown = driftUpdateSamples + static_cast<int>((nextRandom() + 1.0f) * driftUpdateSamples * 0.5f);
        }
        const auto driftStep = driftValue > 0.0f ? 1.0f / static_cast<float>(driftUpdateCountdown + 1) : 0.0f;
        driftCurrent += (driftTarget - driftCurrent) * driftStep;
        driftPhase += static_cast<double>(driftCurrent) * 0.00002;
        if (driftValue == 0.0f)
            driftCurrent = driftTarget = 0.0f;

        const auto wetLeft = frozenSample(0);
        const auto wetRight = frozenSample(1);
        const auto wet = dryWetCurrent * freezeGain;
        const auto outputLeft = left * (1.0f - wet) + wetLeft * wet;
        const auto outputRight = right * (1.0f - wet) + wetRight * wet;
        if (channels > 0)
            buffer.setSample(0, sample, std::isfinite(outputLeft) ? outputLeft : 0.0f);
        if (channels > 1)
            buffer.setSample(1, sample, std::isfinite(outputRight) ? outputRight : 0.0f);

        if (freeze && capturedLength > 0)
        {
            const auto speed = 1.0 + static_cast<double>(driftCurrent) * 0.02 + std::sin(driftPhase) * driftValue * 0.002;
            playbackPosition += speed;
            if (playbackPosition >= capturedLength)
                playbackPosition = std::fmod(playbackPosition, static_cast<double>(capturedLength));
        }
    }
}
}
