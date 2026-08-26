#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cstdint>
#include <vector>

namespace nachgluehen
{
class LivingFreezeEngine
{
public:
    LivingFreezeEngine() = default;

    void prepare(double sampleRate, int maximumBlockSize, int numChannels = 2);
    void reset();
    void setSeed(std::uint32_t seed) noexcept;
    void process(juce::AudioBuffer<float>& buffer, bool freeze, float drift, float dryWet);

    int getCapturedLength() const noexcept { return capturedLength; }
    bool hasCapture() const noexcept { return capturedLength > 0; }

private:
    static constexpr double captureDurationSeconds = 0.6;
    static constexpr double maxPositionDriftMilliseconds = 35.0;
    static constexpr double maxPlaybackDrift = 0.08;
    static constexpr int transitionSamples = 256;
    static constexpr int driftUpdateSamples = 2048;

    float readLinear(const std::vector<float>& source, int length, double position) const noexcept;
    float nextRandom() noexcept;
    void captureRecent();
    float frozenSample(int channel) noexcept;

    double currentSampleRate = 44100.0;
    int maxBlockSize = 0;
    int ringCapacity = 0;
    int ringWrite = 0;
    int recentSamples = 0;
    int capturedLength = 0;
    double playbackPosition = 0.0;
    double maxPositionDriftSamples = 0.0;
    double driftSmoothingCoefficient = 1.0;
    int crossfadeLength = 64;
    int transitionRemaining = 0;
    bool wasFrozen = false;
    float freezeGain = 0.0f;
    float dryWetCurrent = 0.5f;
    float dryWetValue = 0.5f;
    float driftValue = 0.2f;
    float driftCurrent = 0.0f;
    float driftTarget = 0.0f;
    int driftUpdateCountdown = driftUpdateSamples;
    std::uint32_t randomState = 0x4e616368u;
    std::vector<float> recentLeft, recentRight, frozenLeft, frozenRight;
};
}
