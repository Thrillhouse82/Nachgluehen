#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace nachgluehen
{
namespace parameterIds
{
inline constexpr auto freeze = "freeze";
inline constexpr auto drift = "drift";
inline constexpr auto smooth = "smooth";
inline constexpr auto dryWet = "dryWet";
inline constexpr auto outputGain = "outputGain";
}

inline constexpr float outputGainMuteDb = -100.0f;
inline constexpr float outputGainMaximumDb = 12.0f;

inline juce::NormalisableRange<float> createOutputGainRange()
{
    return { outputGainMuteDb, outputGainMaximumDb,
        [](float, float, float normalised)
        {
            normalised = juce::jlimit(0.0f, 1.0f, normalised);
            return normalised <= 0.5f
                ? outputGainMuteDb + normalised * 200.0f
                : (normalised - 0.5f) * 24.0f;
        },
        [](float, float, float value)
        {
            value = juce::jlimit(outputGainMuteDb, outputGainMaximumDb, value);
            return value <= 0.0f
                ? (value - outputGainMuteDb) / 200.0f
                : 0.5f + value / 24.0f;
        } };
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    const auto percentAttributes = []
    {
        return juce::AudioParameterFloatAttributes()
            .withLabel("%")
            .withStringFromValueFunction([](float value, int) { return juce::String(juce::roundToInt(value * 100.0f)); })
            .withValueFromStringFunction([](const juce::String& text) { return text.getFloatValue() * 0.01f; });
    };
    const auto outputGainAttributes = []
    {
        return juce::AudioParameterFloatAttributes()
            .withLabel("dB")
            .withStringFromValueFunction([](float value, int)
            {
                if (value <= outputGainMuteDb + 0.05f)
                    return juce::String("-inf");
                return juce::String(value > 0.0f ? "+" : "") + juce::String(value, 1);
            })
            .withValueFromStringFunction([](const juce::String& text)
            {
                return text.containsIgnoreCase("inf") ? outputGainMuteDb : text.getFloatValue();
            });
    };
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(parameterIds::freeze, "Freeze", false));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::drift, "Drift", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.20f, percentAttributes()));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::smooth, "Smooth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.50f, percentAttributes()));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::dryWet, "Dry/Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.50f, percentAttributes()));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::outputGain, "Output Gain", createOutputGainRange(), 0.0f, outputGainAttributes()));
    return { parameters.begin(), parameters.end() };
}
}
