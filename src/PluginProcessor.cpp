#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

NachgluehenAudioProcessor::NachgluehenAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", nachgluehen::createParameterLayout())
{
}

void NachgluehenAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock, 2);
    outputGain.reset(sampleRate, 0.02);
    outputGain.setCurrentAndTargetValue(1.0f);
    clipHoldDurationSamples = juce::jmax(1, static_cast<int>(std::ceil(sampleRate)));
    clipHoldSamplesRemaining.store(0, std::memory_order_relaxed);
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

    const auto outputGainDb = juce::jlimit(muteOutputGainDb, 12.0f,
        parameters.getRawParameterValue(nachgluehen::parameterIds::outputGain)->load());
    const auto targetGain = outputGainDb <= muteOutputGainDb + 0.05f
        ? 0.0f : juce::Decibels::decibelsToGain(outputGainDb);
    outputGain.setTargetValue(std::isfinite(targetGain) ? targetGain : 0.0f);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto gain = outputGain.getNextValue();
        bool clipped = false;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            const auto output = buffer.getSample(channel, sample) * gain;
            const auto safeOutput = std::isfinite(output) ? output : 0.0f;
            buffer.setSample(channel, sample, safeOutput);
            clipped = clipped || std::abs(safeOutput) >= clipThreshold;
        }
        if (clipped)
            clipHoldSamplesRemaining.store(clipHoldDurationSamples, std::memory_order_relaxed);
        else
        {
            const auto remaining = clipHoldSamplesRemaining.load(std::memory_order_relaxed);
            if (remaining > 0)
                clipHoldSamplesRemaining.store(remaining - 1, std::memory_order_relaxed);
        }
    }
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
