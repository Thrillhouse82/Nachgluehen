#include "PluginProcessor.h"
#include "PluginEditor.h"

NachgluehenAudioProcessor::NachgluehenAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", nachgluehen::createParameterLayout())
{
}

void NachgluehenAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock, 2);
}

void NachgluehenAudioProcessor::releaseResources()
{
    engine.reset();
}

bool NachgluehenAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    const auto input = layouts.getMainInputChannelSet();
    return (output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo()) && output == input;
}

void NachgluehenAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midi);
    const auto freeze = parameters.getRawParameterValue(nachgluehen::parameterIds::freeze)->load() >= 0.5f;
    const auto drift = parameters.getRawParameterValue(nachgluehen::parameterIds::drift)->load();
    const auto dryWet = parameters.getRawParameterValue(nachgluehen::parameterIds::dryWet)->load();
    engine.process(buffer, freeze, drift, dryWet);
}

void NachgluehenAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (const auto state = parameters.copyState(); state.isValid())
        if (const auto xml = state.createXml())
            copyXmlToBinary(*xml, destination);
}

void NachgluehenAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
    engine.reset();
}

juce::AudioProcessorEditor* NachgluehenAudioProcessor::createEditor()
{
    return new NachgluehenAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NachgluehenAudioProcessor();
}
