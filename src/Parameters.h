#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace nachgluehen
{
namespace parameterIds
{
inline constexpr auto freeze = "freeze";
inline constexpr auto drift = "drift";
inline constexpr auto dryWet = "dryWet";
inline constexpr auto outputGain = "outputGain";
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
                return value <= -99.9f ? juce::String("-inf") : juce::String(value, 1);
            })
            .withValueFromStringFunction([](const juce::String& text)
            {
                return text.containsIgnoreCase("inf") ? -100.0f : text.getFloatValue();
            });
    };
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(parameterIds::freeze, "Freeze", false));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::drift, "Drift", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.20f, percentAttributes()));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::dryWet, "Dry/Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.50f, percentAttributes()));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIds::outputGain, "Output Gain", juce::NormalisableRange<float>(-100.0f, 12.0f, 0.1f), 0.0f, outputGainAttributes()));
    return { parameters.begin(), parameters.end() };
}
}
