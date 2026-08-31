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
    static float windowValue(double phase) noexcept;
    static float pitchDriftAmount(float normalizedDrift) noexcept;

    // Read-only diagnostics used by deterministic DSP regression tests.
    float getVoicePlaybackSpeed(int index) const noexcept;
    float getVoiceSpeedTarget(int index) const noexcept;
    float getVoicePitchFactor(int index) const noexcept;
    float getGlobalPlaybackSpeed() const noexcept { return static_cast<float>(globalPlaybackSpeed); }
    float getGlobalPlaybackSpeedTarget() const noexcept { return static_cast<float>(globalPlaybackSpeedTarget); }
    float getVoicePosition(int index) const noexcept;
    float getVoicePositionOffset(int index) const noexcept;
    float getVoicePositionTarget(int index) const noexcept;
    float getVoiceStereoOffset(int index) const noexcept;
    float getVoiceStereoTarget(int index) const noexcept;
    float getVoiceSafeReadMin(int index) const noexcept;
    float getVoiceSafeReadMax(int index) const noexcept;
    float getTextureGainCompensation() const noexcept { return textureGainCompensation; }

private:
    static constexpr int textureVoiceCount = 8;
    static constexpr double captureDurationSeconds = 0.6;
    static constexpr double maxPositionDriftMilliseconds = 55.0;
    static constexpr double movementSmoothingSeconds = 0.45;
    static constexpr double maxPlaybackDrift = 0.08;
    static constexpr double pitchSmoothingSeconds = 1.50;
    static constexpr double textureGainSmoothingSeconds = 0.01;
    static constexpr double textureGainFloor = 0.12;
    static constexpr double textureGainCeiling = 0.55;
    static constexpr double textureEnergyVoiceScale = 0.28;
    static constexpr double maximumGlobalPitchDeviation = 0.055;
    static constexpr double globalPitchRandomWalkStep = 0.009;
    static constexpr double localPitchRandomWalkStep = 0.0004;
    static constexpr double localPitchMaximumDeviation = 0.0045;
    static constexpr double minimumWindowFraction = 0.42;
    static constexpr double maximumWindowFraction = 0.58;
    static constexpr double voiceSpacingFraction = 1.0 / static_cast<double>(textureVoiceCount);
    static constexpr double cycleStartStepFraction = 0.37;
    static constexpr int transitionSamples = 256;
    static constexpr int driftUpdateSamples = 2048;

    struct TextureVoice
    {
        double readPosition = 0.0;
        double playbackSpeed = 1.0;
        double windowPhase = 0.0;
        double windowLength = 1.0;
        double startPosition = 0.0;
        double positionOffset = 0.0;
        double positionTarget = 0.0;
        double localPitchFactor = 1.0;
        double localPitchTarget = 1.0;
        double stereoOffset = 0.0;
        double stereoTarget = 0.0;
        double safeReadMin = 0.0;
        double safeReadMax = 0.0;
        double safeStartMin = 0.0;
        double safeStartMax = 0.0;
        std::uint32_t cycle = 0;
        bool active = false;
    };

    float readLinear(const std::vector<float>& source, int length, double position) const noexcept;
    float nextRandom() noexcept;
    void captureRecent();
    void initializeVoice(TextureVoice& voice, int index) noexcept;
    void restartVoice(TextureVoice& voice, int index) noexcept;
    void updateVoiceSafety(TextureVoice& voice) noexcept;
    void constrainVoiceTargets(TextureVoice& voice) noexcept;
    void updateVoiceTargets() noexcept;
    float renderTexture(int channel) noexcept;

    double currentSampleRate = 44100.0;
    int maxBlockSize = 0;
    int ringCapacity = 0;
    int ringWrite = 0;
    int recentSamples = 0;
    int capturedLength = 0;
    double maxPositionDriftSamples = 0.0;
    double driftSmoothingCoefficient = 1.0;
    double movementSmoothingCoefficient = 1.0;
    double pitchSmoothingCoefficient = 1.0;
    int transitionRemaining = 0;
    bool wasFrozen = false;
    float freezeGain = 0.0f;
    float textureGainCompensation = 1.0f;
    float dryWetCurrent = 0.5f;
    float dryWetValue = 0.5f;
    float driftValue = 0.2f;
    float safetyDriftValue = 0.0f;
    float driftCurrent = 0.0f;
    float driftTarget = 0.0f;
    double globalPlaybackSpeed = 1.0;
    double globalPlaybackSpeedTarget = 1.0;
    int driftUpdateCountdown = driftUpdateSamples;
    std::uint32_t randomState = 0x4e616368u;
    std::array<TextureVoice, textureVoiceCount> voices{};
    std::vector<float> recentLeft, recentRight, frozenLeft, frozenRight;
};
}
