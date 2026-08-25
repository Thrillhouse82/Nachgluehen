#pragma once

#include "LivingFreezeEngine.h"
#include "Parameters.h"
#include <juce_audio_processors/juce_audio_processors.h>

class NachgluehenAudioProcessor final : public juce::AudioProcessor
{
public:
    NachgluehenAudioProcessor();
    ~NachgluehenAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Nachgluehen"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;

    juce::AudioProcessorValueTreeState parameters;
    nachgluehen::LivingFreezeEngine engine;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NachgluehenAudioProcessor)
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
