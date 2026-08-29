#pragma once

#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

class NachgluehenLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    NachgluehenLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;
};

class NachgluehenAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                               private juce::Timer
{
public:
    explicit NachgluehenAudioProcessorEditor(NachgluehenAudioProcessor&);
    ~NachgluehenAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    NachgluehenAudioProcessor& processor;
    NachgluehenLookAndFeel lookAndFeel;
    juce::ToggleButton freezeButton;
    juce::Slider driftSlider, smoothSlider, dryWetSlider, outputGainSlider;
    juce::Label title, driftLabel, smoothLabel, dryWetLabel, outputGainLabel, clipLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> freezeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driftAttachment, smoothAttachment, dryWetAttachment, outputGainAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NachgluehenAudioProcessorEditor)
};
