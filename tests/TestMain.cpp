#include "LivingFreezeEngine.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace
{
class LivingFreezeTests final : public juce::UnitTest
{
public:
    LivingFreezeTests() : UnitTest("Living Freeze Engine", "Nachgluehen") {}

    void runTest() override
    {
        beginTest("Parameter contract and state values");
        NachgluehenAudioProcessor processor;
        auto* freezeParameter = processor.parameters.getParameter(nachgluehen::parameterIds::freeze);
        auto* driftParameter = processor.parameters.getParameter(nachgluehen::parameterIds::drift);
        auto* dryWetParameter = processor.parameters.getParameter(nachgluehen::parameterIds::dryWet);
        auto* outputGainParameter = processor.parameters.getParameter(nachgluehen::parameterIds::outputGain);
        expect(freezeParameter != nullptr && driftParameter != nullptr && dryWetParameter != nullptr && outputGainParameter != nullptr);
        expectWithinAbsoluteError(freezeParameter->getDefaultValue(), 0.0f, 1.0e-6f);
        expectWithinAbsoluteError(driftParameter->getDefaultValue(), 0.20f, 1.0e-6f);
        expectWithinAbsoluteError(dryWetParameter->getDefaultValue(), 0.50f, 1.0e-6f);
        expectWithinAbsoluteError(processor.parameters.getRawParameterValue(nachgluehen::parameterIds::outputGain)->load(), 0.0f, 1.0e-6f);
        expectWithinAbsoluteError(outputGainParameter->getNormalisableRange().start, -100.0f, 1.0e-6f);
        expectWithinAbsoluteError(outputGainParameter->getNormalisableRange().end, 12.0f, 1.0e-6f);
        const auto& outputGainRange = outputGainParameter->getNormalisableRange();
        expectWithinAbsoluteError(outputGainRange.convertFrom0to1(0.0f), -100.0f, 1.0e-6f);
        expectWithinAbsoluteError(outputGainRange.convertFrom0to1(0.5f), 0.0f, 1.0e-6f);
        expectWithinAbsoluteError(outputGainRange.convertFrom0to1(1.0f), 12.0f, 1.0e-6f);
        expectEquals(outputGainParameter->getText(0.0f, 8), juce::String("-inf"));
        expectEquals(outputGainParameter->getText(0.5f, 8), juce::String("0.0"));
        expectEquals(outputGainParameter->getText(1.0f, 8), juce::String("+12.0"));
        for (const auto db : { -100.0f, -50.0f, -12.0f, -3.0f, 0.0f, 6.0f, 12.0f })
            expectWithinAbsoluteError(outputGainRange.convertFrom0to1(outputGainRange.convertTo0to1(db)), db, 1.0e-5f);
        expectWithinAbsoluteError(driftParameter->getNormalisableRange().start, 0.0f, 1.0e-6f);
        expectWithinAbsoluteError(driftParameter->getNormalisableRange().end, 1.0f, 1.0e-6f);
        expectEquals(driftParameter->getText(0.5f, 8), juce::String("50"));
        expectWithinAbsoluteError(driftParameter->getValueForText("50"), 0.5f, 1.0e-6f);
        processor.parameters.getParameter(nachgluehen::parameterIds::freeze)->setValueNotifyingHost(1.0f);
        processor.parameters.getParameter(nachgluehen::parameterIds::drift)->setValueNotifyingHost(0.73f);
        processor.parameters.getParameter(nachgluehen::parameterIds::dryWet)->setValueNotifyingHost(0.11f);
        outputGainParameter->setValueNotifyingHost(outputGainParameter->getNormalisableRange().convertTo0to1(12.0f));
        juce::MemoryBlock state;
        processor.getStateInformation(state);
        NachgluehenAudioProcessor restored;
        restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        expectWithinAbsoluteError(restored.parameters.getRawParameterValue(nachgluehen::parameterIds::drift)->load(), 0.73f, 0.002f);
        expectWithinAbsoluteError(restored.parameters.getRawParameterValue(nachgluehen::parameterIds::dryWet)->load(), 0.11f, 0.002f);
        expect(restored.parameters.getRawParameterValue(nachgluehen::parameterIds::freeze)->load() >= 0.5f);
        expectWithinAbsoluteError(restored.parameters.getRawParameterValue(nachgluehen::parameterIds::outputGain)->load(), 12.0f, 0.11f);
        expect(restored.parameters.getParameter("smooth") == nullptr);

        beginTest("Legacy state with an unknown Smooth value restores safely");
        processor.parameters.state.setProperty("smooth", 0.91f, nullptr);
        juce::MemoryBlock legacyBinary;
        processor.getStateInformation(legacyBinary);
        processor.parameters.state.removeProperty("smooth", nullptr);
        NachgluehenAudioProcessor legacyRestored;
        legacyRestored.setStateInformation(legacyBinary.getData(), static_cast<int>(legacyBinary.getSize()));
        expect(legacyRestored.parameters.getParameter("smooth") == nullptr);
        expectWithinAbsoluteError(legacyRestored.parameters.getRawParameterValue(nachgluehen::parameterIds::drift)->load(), 0.73f, 0.002f);

        beginTest("Editor presents the three equal-size rotary controls without Smooth");
        juce::ScopedJuceInitialiser_GUI gui;
        NachgluehenAudioProcessor editorProcessor;
        std::unique_ptr<juce::AudioProcessorEditor> editor(editorProcessor.createEditor());
        const auto* driftControl = dynamic_cast<const juce::Slider*>(editor->findChildWithID("drift-control"));
        const auto* dryWetControl = dynamic_cast<const juce::Slider*>(editor->findChildWithID("dry-wet-control"));
        const auto* outputControl = dynamic_cast<const juce::Slider*>(editor->findChildWithID("output-gain-control"));
        const auto* smoothControl = editor->findChildWithID("smooth-control");
        expect(driftControl != nullptr && dryWetControl != nullptr && outputControl != nullptr && smoothControl == nullptr);
        if (driftControl != nullptr && dryWetControl != nullptr && outputControl != nullptr)
        {
            expectEquals(driftControl->getBounds().getWidth(), dryWetControl->getBounds().getWidth());
            expectEquals(dryWetControl->getBounds().getWidth(), outputControl->getBounds().getWidth());
            expectEquals(driftControl->getBounds().getHeight(), dryWetControl->getBounds().getHeight());
            expectEquals(dryWetControl->getBounds().getHeight(), outputControl->getBounds().getHeight());
        }

        beginTest("Output Gain handles host automation, staging, and finite output");
        NachgluehenAudioProcessor gainProcessor;
        gainProcessor.prepareToPlay(48000.0, 128);
        auto* gainParameter = gainProcessor.parameters.getParameter(nachgluehen::parameterIds::outputGain);
        const auto setOutputGain = [&gainParameter](float db)
        {
            gainParameter->setValueNotifyingHost(gainParameter->getNormalisableRange().convertTo0to1(db));
        };
        juce::MidiBuffer noMidi;
        juce::AudioBuffer<float> gainBuffer(2, 128);
        for (int channel = 0; channel < gainBuffer.getNumChannels(); ++channel)
            gainBuffer.clear(channel, 0, gainBuffer.getNumSamples());
        for (int sample = 0; sample < gainBuffer.getNumSamples(); ++sample)
        {
            gainBuffer.setSample(0, sample, 0.25f);
            gainBuffer.setSample(1, sample, -0.25f);
        }
        gainProcessor.processBlock(gainBuffer, noMidi);
        expectWithinAbsoluteError(gainBuffer.getSample(0, 64), 0.25f, 1.0e-6f);

        setOutputGain(12.0f);
        float previousGainSample = gainBuffer.getSample(0, 127);
        float maximumGainStep = 0.0f;
        for (int block = 0; block < 10; ++block)
        {
            for (int sample = 0; sample < gainBuffer.getNumSamples(); ++sample)
            {
                gainBuffer.setSample(0, sample, 0.25f);
                gainBuffer.setSample(1, sample, -0.25f);
            }
            gainProcessor.processBlock(gainBuffer, noMidi);
            for (int sample = 0; sample < gainBuffer.getNumSamples(); ++sample)
            {
                expect(std::isfinite(gainBuffer.getSample(0, sample)));
                maximumGainStep = juce::jmax(maximumGainStep, std::abs(gainBuffer.getSample(0, sample) - previousGainSample));
                previousGainSample = gainBuffer.getSample(0, sample);
            }
        }
        expectLessThan(maximumGainStep, 0.01f);
        expectWithinAbsoluteError(gainBuffer.getSample(0, 127), 0.25f * juce::Decibels::decibelsToGain(12.0f), 0.002f);

        setOutputGain(6.0f);
        for (int block = 0; block < 10; ++block)
        {
            for (int sample = 0; sample < gainBuffer.getNumSamples(); ++sample)
            {
                gainBuffer.setSample(0, sample, 0.25f);
                gainBuffer.setSample(1, sample, -0.25f);
            }
            gainProcessor.processBlock(gainBuffer, noMidi);
        }
        expectWithinAbsoluteError(gainBuffer.getSample(0, 127), 0.25f * juce::Decibels::decibelsToGain(6.0f), 0.002f);

        setOutputGain(-100.0f);
        for (int block = 0; block < 10; ++block)
        {
            gainBuffer.applyGain(0.0f);
            for (int sample = 0; sample < gainBuffer.getNumSamples(); ++sample)
            {
                gainBuffer.setSample(0, sample, 0.25f);
                gainBuffer.setSample(1, sample, -0.25f);
            }
            gainProcessor.processBlock(gainBuffer, noMidi);
        }
        expectWithinAbsoluteError(gainBuffer.getSample(0, 127), 0.0f, 1.0e-6f);

        beginTest("Post-gain clip hold uses the threshold and expires after one second");
        NachgluehenAudioProcessor clipProcessor;
        clipProcessor.prepareToPlay(48000.0, 128);
        juce::AudioBuffer<float> clipBuffer(2, 128);
        clipBuffer.clear();
        for (int sample = 0; sample < clipBuffer.getNumSamples(); ++sample)
        {
            clipBuffer.setSample(0, sample, 0.98f);
            clipBuffer.setSample(1, sample, -0.98f);
        }
        clipProcessor.processBlock(clipBuffer, noMidi);
        expect(!clipProcessor.isClipHoldActive());
        clipBuffer.clear();
        for (int sample = 0; sample < clipBuffer.getNumSamples(); ++sample)
            clipBuffer.setSample(0, sample, 0.99f);
        clipProcessor.processBlock(clipBuffer, noMidi);
        expect(clipProcessor.isClipHoldActive());
        int remainingSafeSamples = 47999;
        while (remainingSafeSamples > 0)
        {
            clipBuffer.clear();
            const auto count = juce::jmin(remainingSafeSamples, clipBuffer.getNumSamples());
            juce::AudioBuffer<float> shortBuffer(2, count);
            shortBuffer.clear();
            clipProcessor.processBlock(shortBuffer, noMidi);
            remainingSafeSamples -= count;
        }
        expect(clipProcessor.isClipHoldActive());
        juce::AudioBuffer<float> finalSafeSample(2, 1);
        finalSafeSample.clear();
        clipProcessor.processBlock(finalSafeSample, noMidi);
        expect(!clipProcessor.isClipHoldActive());

        beginTest("Defaults and dry passthrough");
        nachgluehen::LivingFreezeEngine engine;
        engine.prepare(48000.0, 128);
        juce::AudioBuffer<float> buffer(2, 128);
        buffer.clear();
        for (int i = 0; i < 128; ++i) { buffer.setSample(0, i, 0.25f); buffer.setSample(1, i, -0.25f); }
        engine.process(buffer, false, 0.2f, 0.0f);
        expectWithinAbsoluteError(buffer.getSample(0, 64), 0.25f, 1.0e-6f);
        expectWithinAbsoluteError(buffer.getSample(1, 64), -0.25f, 1.0e-6f);

        beginTest("Pitch drift mapping is zero-ended and nonlinear");
        expectWithinAbsoluteError(nachgluehen::LivingFreezeEngine::pitchDriftAmount(0.0f), 0.0f, 1.0e-6f);
        expectWithinAbsoluteError(nachgluehen::LivingFreezeEngine::pitchDriftAmount(1.0f), 1.0f, 1.0e-6f);
        const auto lowPitchAmount = nachgluehen::LivingFreezeEngine::pitchDriftAmount(0.2f);
        expect(lowPitchAmount < 0.2f * nachgluehen::LivingFreezeEngine::pitchDriftAmount(1.0f));
        expect(nachgluehen::LivingFreezeEngine::pitchDriftAmount(0.5f) > lowPitchAmount);
        expectGreaterThan(nachgluehen::LivingFreezeEngine::pitchDriftAmount(0.5f), 0.20f);

        beginTest("Mid Drift has measurable strengthened pitch and position response");
        nachgluehen::LivingFreezeEngine midDriftEngine;
        midDriftEngine.prepare(1000.0, 64);
        midDriftEngine.setSeed(9876);
        juce::AudioBuffer<float> midDriftBuffer(2, 64);
        for (int block = 0; block < 12; ++block)
        {
            for (int sample = 0; sample < 64; ++sample)
            {
                const auto value = std::sin((block * 64 + sample) * 0.023f);
                midDriftBuffer.setSample(0, sample, value);
                midDriftBuffer.setSample(1, sample, value);
            }
            midDriftEngine.process(midDriftBuffer, false, 0.0f, 0.0f);
        }
        midDriftBuffer.clear();
        midDriftEngine.process(midDriftBuffer, true, 0.5f, 1.0f);
        float largestMidPositionTarget = 0.0f;
        for (int index = 0; index < 8; ++index)
            largestMidPositionTarget = juce::jmax(largestMidPositionTarget,
                                                  std::abs(midDriftEngine.getVoicePositionTarget(index)));
        expectGreaterThan(largestMidPositionTarget, 5.0f);

        beginTest("Pitch drift changes remain finite and continuous");
        nachgluehen::LivingFreezeEngine pitchTransitionEngine;
        pitchTransitionEngine.prepare(48000.0, 256);
        juce::AudioBuffer<float> pitchBuffer(2, 256);
        for (int block = 0; block < 16; ++block)
        {
            for (int i = 0; i < pitchBuffer.getNumSamples(); ++i)
            {
                const auto value = std::sin((block * pitchBuffer.getNumSamples() + i) * 0.021f);
                pitchBuffer.setSample(0, i, value);
                pitchBuffer.setSample(1, i, value);
            }
            pitchTransitionEngine.process(pitchBuffer, false, 0.0f, 0.0f);
        }
        pitchBuffer.clear();
        pitchTransitionEngine.process(pitchBuffer, true, 0.1f, 1.0f);
        float previousPitchSample = pitchBuffer.getSample(0, pitchBuffer.getNumSamples() - 1);
        float maximumPitchStep = 0.0f;
        for (int block = 0; block < 80; ++block)
        {
            pitchBuffer.clear();
            const auto driftValue = block < 8 ? 0.1f : 1.0f;
            pitchTransitionEngine.process(pitchBuffer, true, driftValue, 1.0f);
            for (int i = 0; i < pitchBuffer.getNumSamples(); ++i)
            {
                const auto value = pitchBuffer.getSample(0, i);
                expect(std::isfinite(value));
                maximumPitchStep = juce::jmax(maximumPitchStep, std::abs(value - previousPitchSample));
                previousPitchSample = value;
            }
        }
        expectLessThan(maximumPitchStep, 0.75f);

        beginTest("Capture uses a bounded window immediately before Freeze");
        nachgluehen::LivingFreezeEngine captureEngine;
        captureEngine.prepare(48000.0, 128);
        juce::AudioBuffer<float> captureBuffer(2, 128);
        for (int block = 0; block < 160; ++block)
        {
            captureBuffer.clear();
            for (int i = 0; i < 128; ++i)
            {
                captureBuffer.setSample(0, i, -0.75f);
                captureBuffer.setSample(1, i, -0.75f);
            }
            captureEngine.process(captureBuffer, false, 0.0f, 0.0f);
        }
        for (int block = 0; block < 240; ++block)
        {
            captureBuffer.clear();
            for (int i = 0; i < 128; ++i)
            {
                captureBuffer.setSample(0, i, 0.35f);
                captureBuffer.setSample(1, i, 0.35f);
            }
            captureEngine.process(captureBuffer, false, 0.0f, 0.0f);
        }
        captureBuffer.clear();
        captureEngine.process(captureBuffer, true, 0.0f, 1.0f);
        expectEquals(captureEngine.getCapturedLength(), 28800);
        for (int block = 0; block < 4; ++block)
        {
            captureBuffer.clear();
            captureEngine.process(captureBuffer, true, 0.0f, 1.0f);
        }
        expectGreaterThan(std::abs(captureBuffer.getSample(0, 127)), 0.10f);
        expectGreaterThan(std::abs(captureBuffer.getSample(1, 127)), 0.10f);

        beginTest("Capture survives changed input");
        for (int block = 0; block < 8; ++block)
        {
            buffer.clear();
            for (int i = 0; i < 128; ++i) buffer.setSample(0, i, 0.2f);
            engine.process(buffer, false, 0.0f, 0.0f);
        }
        buffer.clear();
        engine.process(buffer, true, 0.0f, 1.0f);
        expect(engine.hasCapture());
        buffer.clear();
        engine.process(buffer, true, 0.0f, 1.0f);
        expectGreaterThan(std::abs(buffer.getSample(0, 64)), 0.01f);

        beginTest("Repeated freeze playback remains dense and finite");
        nachgluehen::LivingFreezeEngine continuityEngine;
        continuityEngine.prepare(48000.0, 128);
        for (int block = 0; block < 8; ++block)
        {
            buffer.clear();
            for (int i = 0; i < 128; ++i) { buffer.setSample(0, i, 0.4f); buffer.setSample(1, i, 0.4f); }
            continuityEngine.process(buffer, false, 0.0f, 0.0f);
        }
        buffer.clear();
        continuityEngine.process(buffer, true, 0.0f, 1.0f);
        float previous = buffer.getSample(0, 0);
        int silentBlocks = 0;
        float maximumStep = 0.0f;
        for (int block = 0; block < 40; ++block)
        {
            buffer.clear();
            continuityEngine.process(buffer, true, 0.0f, 1.0f);
            float blockPeak = 0.0f;
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                const auto sample = buffer.getSample(0, i);
                expect(std::isfinite(sample));
                blockPeak = juce::jmax(blockPeak, std::abs(sample));
                maximumStep = juce::jmax(maximumStep, std::abs(sample - previous));
                previous = sample;
            }
            if (blockPeak < 0.05f)
                ++silentBlocks;
        }
        expectEquals(silentBlocks, 0);
        expectLessThan(maximumStep, 0.5f, "maximum step = " + juce::String(maximumStep));

        beginTest("Texture playback does not repeat a marked impulse at capture length");
        nachgluehen::LivingFreezeEngine impulseEngine;
        impulseEngine.prepare(1000.0, 64);
        juce::AudioBuffer<float> impulseBuffer(2, 64);
        for (int block = 0; block < 10; ++block)
        {
            impulseBuffer.clear();
            if (block == 2)
                impulseBuffer.setSample(0, 0, 1.0f);
            impulseEngine.process(impulseBuffer, false, 0.0f, 0.0f);
        }
        juce::AudioBuffer<float> impulseOutput(2, 2000);
        impulseOutput.clear();
        impulseEngine.process(impulseOutput, true, 0.0f, 1.0f);
        double repeatedDifference = 0.0;
        for (int i = 0; i < 1200; ++i)
            repeatedDifference += std::abs(impulseOutput.getSample(0, i) - impulseOutput.getSample(0, i + 600));
        expectGreaterThan(repeatedDifference, 0.001);

        beginTest("Window has safe continuous endpoints");
        expectWithinAbsoluteError(nachgluehen::LivingFreezeEngine::windowValue(0.0), 0.0f, 1.0e-6f);
        expectWithinAbsoluteError(nachgluehen::LivingFreezeEngine::windowValue(1.0), 0.0f, 1.0e-6f);
        float previousWindow = 0.0f;
        for (int i = 0; i <= 100; ++i)
        {
            const auto value = nachgluehen::LivingFreezeEngine::windowValue(i / 100.0);
            expect(value >= 0.0f);
            expect(std::abs(value - previousWindow) < 0.04f);
            previousWindow = value;
        }

        beginTest("Buffer boundaries remain click-free at high Drift");
        nachgluehen::LivingFreezeEngine boundaryEngine;
        boundaryEngine.prepare(1000.0, 64);
        juce::AudioBuffer<float> boundaryInput(2, 64);
        for (int block = 0; block < 10; ++block)
        {
            for (int i = 0; i < 64; ++i)
            {
                const auto position = block * 64 + i;
                const auto value = -1.0f + 2.0f * static_cast<float>(position) / 639.0f;
                boundaryInput.setSample(0, i, value);
                boundaryInput.setSample(1, i, value);
            }
            boundaryEngine.process(boundaryInput, false, 0.0f, 0.0f);
        }
        boundaryInput.clear();
        boundaryEngine.process(boundaryInput, true, 1.0f, 1.0f);
        float boundaryPrevious = boundaryInput.getSample(0, 0);
        float boundaryMaximumStep = 0.0f;
        for (int block = 0; block < 120; ++block)
        {
            boundaryInput.clear();
            boundaryEngine.process(boundaryInput, true, 1.0f, 1.0f);
            for (int i = 0; i < boundaryInput.getNumSamples(); ++i)
            {
                const auto value = boundaryInput.getSample(0, i);
                expect(std::isfinite(value));
                boundaryMaximumStep = juce::jmax(boundaryMaximumStep, std::abs(value - boundaryPrevious));
                boundaryPrevious = value;
            }
        }
        expectLessThan(boundaryMaximumStep, 0.75f);

        beginTest("Window gain is not neutralized by texture mixing");
        nachgluehen::LivingFreezeEngine gainEngine;
        gainEngine.prepare(48000.0, 256);
        juce::AudioBuffer<float> gainInput(2, 256);
        for (int block = 0; block < 8; ++block)
        {
            gainInput.clear();
            for (int i = 0; i < gainInput.getNumSamples(); ++i)
            {
                gainInput.setSample(0, i, 0.5f);
                gainInput.setSample(1, i, 0.5f);
            }
            gainEngine.process(gainInput, false, 0.0f, 0.0f);
        }
        gainInput.clear();
        gainEngine.process(gainInput, true, 0.0f, 1.0f);
        float gainMinimum = 1.0f;
        float gainMaximum = 0.0f;
        for (int block = 0; block < 120; ++block)
        {
            gainInput.clear();
            gainEngine.process(gainInput, true, 0.0f, 1.0f);
            for (int i = 0; i < gainInput.getNumSamples(); ++i)
            {
                const auto value = std::abs(gainInput.getSample(0, i));
                gainMinimum = juce::jmin(gainMinimum, value);
                gainMaximum = juce::jmax(gainMaximum, value);
            }
        }
        expectGreaterThan(gainMaximum, gainMinimum + 0.02f);
        expectLessThan(gainMaximum, 1.25f);

        beginTest("Energy-aware texture gain remains bounded through restarts");
        nachgluehen::LivingFreezeEngine energyEngine;
        energyEngine.prepare(48000.0, 256);
        juce::AudioBuffer<float> energyBuffer(2, 256);
        for (int block = 0; block < 32; ++block)
        {
            for (int i = 0; i < energyBuffer.getNumSamples(); ++i)
            {
                const auto sample = std::sin((block * energyBuffer.getNumSamples() + i) * 0.047f);
                energyBuffer.setSample(0, i, sample);
                energyBuffer.setSample(1, i, sample);
            }
            energyEngine.process(energyBuffer, false, 0.0f, 0.0f);
        }
        float energyRms = 0.0f;
        float energyPeak = 0.0f;
        for (int block = 0; block < 360; ++block)
        {
            energyBuffer.clear();
            energyEngine.process(energyBuffer, true, block % 3 == 0 ? 1.0f : 0.2f, 1.0f);
            for (int i = 0; i < energyBuffer.getNumSamples(); ++i)
            {
                const auto sample = energyBuffer.getSample(0, i);
                expect(std::isfinite(sample));
                energyRms += sample * sample;
                energyPeak = juce::jmax(energyPeak, std::abs(sample));
            }
            expect(energyEngine.getTextureGainCompensation() >= 0.12f - 1.0e-4f);
            expect(energyEngine.getTextureGainCompensation() <= 0.55f + 1.0e-4f);
        }
        energyRms = std::sqrt(energyRms / (360.0f * energyBuffer.getNumSamples()));
        expectGreaterThan(energyRms, 0.035f);
        expectLessThan(energyPeak, 1.25f);

        beginTest("Texture RMS and peak stay controlled across correlated and decorrelated drift");
        for (const auto drift : { 0.0f, 0.2f, 1.0f })
        {
            nachgluehen::LivingFreezeEngine levelEngine;
            levelEngine.prepare(48000.0, 256);
            levelEngine.setSeed(77);
            juce::AudioBuffer<float> levelBuffer(2, 256);
            for (int block = 0; block < 40; ++block)
            {
                for (int i = 0; i < levelBuffer.getNumSamples(); ++i)
                {
                    const auto phase = static_cast<float>(block * levelBuffer.getNumSamples() + i);
                    levelBuffer.setSample(0, i, std::sin(phase * 0.037f));
                    levelBuffer.setSample(1, i, std::sin(phase * 0.037f + 0.71f));
                }
                levelEngine.process(levelBuffer, false, 0.0f, 0.0f);
            }
            double rmsSum = 0.0;
            float peak = 0.0f;
            for (int block = 0; block < 180; ++block)
            {
                levelBuffer.clear();
                levelEngine.process(levelBuffer, true, drift, 1.0f);
                for (int i = 0; i < levelBuffer.getNumSamples(); ++i)
                {
                    const auto sample = levelBuffer.getSample(0, i);
                    expect(std::isfinite(sample));
                    rmsSum += sample * sample;
                    peak = juce::jmax(peak, std::abs(sample));
                }
            }
            const auto rms = std::sqrt(rmsSum / (180.0 * levelBuffer.getNumSamples()));
            expectGreaterThan(static_cast<float>(rms), 0.03f);
            expectLessThan(peak, 1.35f);
        }

        beginTest("Bounded random-walk targets and speed glides stay continuous");
        nachgluehen::LivingFreezeEngine randomWalkEngine;
        randomWalkEngine.prepare(48000.0, 4096);
        randomWalkEngine.setSeed(1234);
        juce::AudioBuffer<float> randomWalkBuffer(2, 4096);
        for (int i = 0; i < randomWalkBuffer.getNumSamples(); ++i)
        {
            const auto sample = std::sin(i * 0.029f);
            randomWalkBuffer.setSample(0, i, sample);
            randomWalkBuffer.setSample(1, i, sample);
        }
        randomWalkEngine.process(randomWalkBuffer, false, 0.0f, 0.0f);
        float previousTarget = 1.0f;
        float previousSpeed = 1.0f;
        for (int block = 0; block < 36; ++block)
        {
            randomWalkBuffer.clear();
            randomWalkEngine.process(randomWalkBuffer, true, 1.0f, 1.0f);
            const auto target = randomWalkEngine.getVoiceSpeedTarget(0);
            expect(std::abs(target - previousTarget) <= 0.0181f);
            previousTarget = target;
            const auto speed = randomWalkEngine.getVoicePlaybackSpeed(0);
            expect(std::isfinite(speed));
            expectLessThan(std::abs(speed - previousSpeed), 0.01f);
            previousSpeed = speed;
        }

        beginTest("Shared pitch drift stays coherent, bounded, and restart-continuous");
        nachgluehen::LivingFreezeEngine coherenceEngine;
        coherenceEngine.prepare(1000.0, 64);
        coherenceEngine.setSeed(2026);
        juce::AudioBuffer<float> coherenceBuffer(2, 64);
        for (int block = 0; block < 12; ++block)
        {
            for (int sample = 0; sample < 64; ++sample)
            {
                const auto value = std::sin((block * 64 + sample) * 0.043f);
                coherenceBuffer.setSample(0, sample, value);
                coherenceBuffer.setSample(1, sample, value);
            }
            coherenceEngine.process(coherenceBuffer, false, 0.0f, 0.0f);
        }
        coherenceBuffer.clear();
        coherenceEngine.process(coherenceBuffer, true, 1.0f, 1.0f);
        float previousGlobalSpeed = coherenceEngine.getGlobalPlaybackSpeed();
        std::array<float, 8> previousVoiceSpeeds{};
        for (int index = 0; index < 8; ++index)
            previousVoiceSpeeds[static_cast<size_t>(index)] = coherenceEngine.getVoicePlaybackSpeed(index);
        for (int block = 0; block < 80; ++block)
        {
            coherenceBuffer.clear();
            coherenceEngine.process(coherenceBuffer, true, 1.0f, 1.0f);
            const auto globalSpeed = coherenceEngine.getGlobalPlaybackSpeed();
            expect(std::isfinite(globalSpeed));
            expectLessThan(std::abs(globalSpeed - previousGlobalSpeed), 0.01f);
            previousGlobalSpeed = globalSpeed;
            for (int index = 0; index < 8; ++index)
            {
                const auto speed = coherenceEngine.getVoicePlaybackSpeed(index);
                const auto factor = coherenceEngine.getVoicePitchFactor(index);
                expect(std::isfinite(speed) && std::isfinite(factor));
                expectLessThan(std::abs(factor - 1.0f), 0.005f);
                expectLessThan(std::abs(speed / globalSpeed - 1.0f), 0.005f);
                expectLessThan(std::abs(speed - previousVoiceSpeeds[static_cast<size_t>(index)]), 0.01f);
                previousVoiceSpeeds[static_cast<size_t>(index)] = speed;
            }
        }

        beginTest("Zero Drift keeps global and local pitch at unity");
        for (int block = 0; block < 12; ++block)
        {
            coherenceBuffer.clear();
            coherenceEngine.process(coherenceBuffer, true, 0.0f, 1.0f);
            expectWithinAbsoluteError(coherenceEngine.getGlobalPlaybackSpeed(), 1.0f, 1.0e-6f);
            expectWithinAbsoluteError(coherenceEngine.getGlobalPlaybackSpeedTarget(), 1.0f, 1.0e-6f);
            for (int index = 0; index < 8; ++index)
            {
                expectWithinAbsoluteError(coherenceEngine.getVoicePlaybackSpeed(index), 1.0f, 1.0e-6f);
                expectWithinAbsoluteError(coherenceEngine.getVoicePitchFactor(index), 1.0f, 1.0e-6f);
                expectWithinAbsoluteError(coherenceEngine.getVoiceSpeedTarget(index), 1.0f, 1.0e-6f);
                expectWithinAbsoluteError(coherenceEngine.getVoicePositionOffset(index), 0.0f, 1.0e-6f);
                expectWithinAbsoluteError(coherenceEngine.getVoicePositionTarget(index), 0.0f, 1.0e-6f);
                expectWithinAbsoluteError(coherenceEngine.getVoiceStereoOffset(index), 0.0f, 1.0e-6f);
                expectWithinAbsoluteError(coherenceEngine.getVoiceStereoTarget(index), 0.0f, 1.0e-6f);
            }
        }

        beginTest("Maximum Drift targets stay within the strengthened pitch and position bounds");
        nachgluehen::LivingFreezeEngine boundedDriftEngine;
        boundedDriftEngine.prepare(1000.0, 64);
        boundedDriftEngine.setSeed(31415);
        juce::AudioBuffer<float> boundedDriftBuffer(2, 64);
        for (int block = 0; block < 12; ++block)
        {
            for (int sample = 0; sample < 64; ++sample)
            {
                const auto value = std::sin((block * 64 + sample) * 0.041f);
                boundedDriftBuffer.setSample(0, sample, value);
                boundedDriftBuffer.setSample(1, sample, value);
            }
            boundedDriftEngine.process(boundedDriftBuffer, false, 0.0f, 0.0f);
        }
        for (int block = 0; block < 120; ++block)
        {
            boundedDriftBuffer.clear();
            boundedDriftEngine.process(boundedDriftBuffer, true, 1.0f, 1.0f);
            expectLessThan(std::abs(boundedDriftEngine.getGlobalPlaybackSpeedTarget() - 1.0f), 0.0551f);
            expectLessThan(std::abs(boundedDriftEngine.getGlobalPlaybackSpeed() - 1.0f), 0.0551f);
            for (int index = 0; index < 8; ++index)
            {
                expectLessThan(std::abs(boundedDriftEngine.getVoicePitchFactor(index) - 1.0f), 0.0046f);
                expectLessThan(std::abs(boundedDriftEngine.getVoiceSpeedTarget(index)
                                        / boundedDriftEngine.getGlobalPlaybackSpeedTarget() - 1.0f), 0.0046f);
                expectLessThan(std::abs(boundedDriftEngine.getVoicePositionTarget(index)), 55.1f);
                expect(boundedDriftEngine.getVoicePosition(index) >= boundedDriftEngine.getVoiceSafeReadMin(index));
                expect(boundedDriftEngine.getVoicePosition(index) <= boundedDriftEngine.getVoiceSafeReadMax(index));
            }
        }

        beginTest("Pitch targets glide instead of immediately changing playback speed");
        nachgluehen::LivingFreezeEngine pitchGlideEngine;
        pitchGlideEngine.prepare(1000.0, 1);
        pitchGlideEngine.setSeed(2718);
        juce::AudioBuffer<float> pitchGlideBuffer(2, 1);
        for (int sample = 0; sample < 640; ++sample)
        {
            const auto value = std::sin(sample * 0.037f);
            pitchGlideBuffer.setSample(0, 0, value);
            pitchGlideBuffer.setSample(1, 0, value);
            pitchGlideEngine.process(pitchGlideBuffer, false, 0.0f, 0.0f);
        }
        pitchGlideBuffer.clear();
        pitchGlideEngine.process(pitchGlideBuffer, true, 1.0f, 1.0f);
        const auto firstGlideSpeed = pitchGlideEngine.getGlobalPlaybackSpeed();
        const auto firstGlideTarget = pitchGlideEngine.getGlobalPlaybackSpeedTarget();
        expectGreaterThan(std::abs(firstGlideTarget - 1.0f), 1.0e-5f);
        expectLessThan(std::abs(firstGlideSpeed - 1.0f), std::abs(firstGlideTarget - 1.0f));
        expectGreaterThan(std::abs(firstGlideTarget - firstGlideSpeed), 1.0e-5f);

        beginTest("Long renders remain finite, safe, continuous, and level-stable across Drift");
        std::array<double, 5> driftRms{};
        const std::array<float, 5> driftValues { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
        for (size_t driftIndex = 0; driftIndex < driftValues.size(); ++driftIndex)
        {
            nachgluehen::LivingFreezeEngine renderEngine;
            renderEngine.prepare(48000.0, 256);
            renderEngine.setSeed(100 + static_cast<std::uint32_t>(driftIndex));
            juce::AudioBuffer<float> renderBuffer(2, 256);
            for (int block = 0; block < 40; ++block)
            {
                for (int sample = 0; sample < renderBuffer.getNumSamples(); ++sample)
                {
                    const auto phase = static_cast<float>(block * renderBuffer.getNumSamples() + sample);
                    const auto value = std::sin(phase * 0.037f) + 0.18f * std::sin(phase * 0.013f);
                    renderBuffer.setSample(0, sample, value);
                    renderBuffer.setSample(1, sample, value);
                }
                renderEngine.process(renderBuffer, false, 0.0f, 0.0f);
            }
            double energy = 0.0;
            float previousSample = 0.0f;
            float maximumStep = 0.0f;
            for (int block = 0; block < 240; ++block)
            {
                renderBuffer.clear();
                renderEngine.process(renderBuffer, true, driftValues[driftIndex], 1.0f);
                for (int sample = 0; sample < renderBuffer.getNumSamples(); ++sample)
                {
                    const auto value = renderBuffer.getSample(0, sample);
                    expect(std::isfinite(value));
                    energy += value * value;
                    maximumStep = juce::jmax(maximumStep, std::abs(value - previousSample));
                    previousSample = value;
                }
                for (int voice = 0; voice < 8; ++voice)
                {
                    expect(std::isfinite(renderEngine.getVoicePlaybackSpeed(voice)));
                    expect(std::isfinite(renderEngine.getVoicePosition(voice)));
                    expect(renderEngine.getVoicePosition(voice) >= renderEngine.getVoiceSafeReadMin(voice));
                    expect(renderEngine.getVoicePosition(voice) <= renderEngine.getVoiceSafeReadMax(voice));
                }
            }
            driftRms[driftIndex] = std::sqrt(energy / (240.0 * renderBuffer.getNumSamples()));
            expectGreaterThan(static_cast<float>(driftRms[driftIndex]), 0.03f);
            expectLessThan(maximumStep, 0.75f);
        }
        const auto [minimumRms, maximumRms] = std::minmax_element(driftRms.begin(), driftRms.end());
        expectGreaterThan(static_cast<float>(*minimumRms), static_cast<float>(*maximumRms * 0.5));

        beginTest("Low Drift pitch range is much smaller than maximum Drift");
        const auto maximumPitchDeviation = [](float drift)
        {
            nachgluehen::LivingFreezeEngine pitchRangeEngine;
            pitchRangeEngine.prepare(1000.0, 64);
            pitchRangeEngine.setSeed(88);
            juce::AudioBuffer<float> pitchRangeBuffer(2, 64);
            for (int block = 0; block < 12; ++block)
            {
                for (int sample = 0; sample < 64; ++sample)
                {
                    const auto value = std::sin((block * 64 + sample) * 0.029f);
                    pitchRangeBuffer.setSample(0, sample, value);
                    pitchRangeBuffer.setSample(1, sample, value);
                }
                pitchRangeEngine.process(pitchRangeBuffer, false, 0.0f, 0.0f);
            }
            float maximumDeviation = 0.0f;
            for (int block = 0; block < 100; ++block)
            {
                pitchRangeBuffer.clear();
                pitchRangeEngine.process(pitchRangeBuffer, true, drift, 1.0f);
                maximumDeviation = juce::jmax(maximumDeviation, std::abs(pitchRangeEngine.getGlobalPlaybackSpeed() - 1.0f));
            }
            return maximumDeviation;
        };
        const auto lowPitchDeviation = maximumPitchDeviation(0.2f);
        const auto highPitchDeviation = maximumPitchDeviation(1.0f);
        expectLessThan(lowPitchDeviation, highPitchDeviation * 0.2f);

        beginTest("Active drift changes move read positions without a jump");
        nachgluehen::LivingFreezeEngine driftChangeEngine;
        driftChangeEngine.prepare(48000.0, 1);
        juce::AudioBuffer<float> driftChangeBuffer(2, 1);
        for (int i = 0; i < 4096; ++i)
        {
            const auto sample = std::sin(i * 0.071f);
            driftChangeBuffer.setSample(0, 0, sample);
            driftChangeBuffer.setSample(1, 0, sample);
            driftChangeEngine.process(driftChangeBuffer, false, 0.0f, 0.0f);
        }
        driftChangeBuffer.clear();
        driftChangeEngine.process(driftChangeBuffer, true, 0.0f, 1.0f);
        auto previousPosition = driftChangeEngine.getVoicePosition(0);
        float previousOutput = driftChangeBuffer.getSample(0, 0);
        for (int sample = 0; sample < 512; ++sample)
        {
            driftChangeBuffer.clear();
            const auto drift = sample < 128 ? 0.0f : 1.0f;
            driftChangeEngine.process(driftChangeBuffer, true, drift, 1.0f);
            const auto position = driftChangeEngine.getVoicePosition(0);
            expectLessThan(std::abs(position - previousPosition), 1.2f);
            previousPosition = position;
            const auto output = driftChangeBuffer.getSample(0, 0);
            expect(std::isfinite(output));
            expectLessThan(std::abs(output - previousOutput), 0.75f);
            previousOutput = output;
        }

        beginTest("Long high-drift rendering remains finite and dense");
        nachgluehen::LivingFreezeEngine longEngine;
        longEngine.prepare(48000.0, 256);
        juce::AudioBuffer<float> longInput(2, 256);
        for (int block = 0; block < 16; ++block)
        {
            for (int i = 0; i < longInput.getNumSamples(); ++i)
            {
                const auto value = std::sin((block * longInput.getNumSamples() + i) * 0.031f);
                longInput.setSample(0, i, value);
                longInput.setSample(1, i, value);
            }
            longEngine.process(longInput, false, 0.0f, 0.0f);
        }
        float longPrevious = 0.0f;
        float longMaximumStep = 0.0f;
        int longSilentBlocks = 0;
        for (int block = 0; block < 400; ++block)
        {
            longInput.clear();
            longEngine.process(longInput, true, 1.0f, 1.0f);
            float blockPeak = 0.0f;
            for (int i = 0; i < longInput.getNumSamples(); ++i)
            {
                const auto value = longInput.getSample(0, i);
                expect(std::isfinite(value));
                blockPeak = juce::jmax(blockPeak, std::abs(value));
                longMaximumStep = juce::jmax(longMaximumStep, std::abs(value - longPrevious));
                longPrevious = value;
            }
            if (blockPeak < 0.01f)
                ++longSilentBlocks;
        }
        expectEquals(longSilentBlocks, 0);
        expectLessThan(longMaximumStep, 0.75f);

        beginTest("Seeded drift is deterministic and finite");
        nachgluehen::LivingFreezeEngine a, b;
        a.prepare(48000.0, 128); b.prepare(48000.0, 128); a.setSeed(42); b.setSeed(42);
        juce::AudioBuffer<float> sourceA(2, 512), sourceB(2, 512);
        for (int i = 0; i < 512; ++i) { const auto v = std::sin(i * 0.03f); sourceA.setSample(0, i, v); sourceA.setSample(1, i, v); sourceB.setSample(0, i, v); sourceB.setSample(1, i, v); }
        a.process(sourceA, false, 0.0f, 0.0f); b.process(sourceB, false, 0.0f, 0.0f);
        sourceA.clear(); sourceB.clear(); a.process(sourceA, true, 0.7f, 1.0f); b.process(sourceB, true, 0.7f, 1.0f);
        for (int i = 0; i < 512; ++i) { expect(std::isfinite(sourceA.getSample(0, i))); expectWithinAbsoluteError(sourceA.getSample(0, i), sourceB.getSample(0, i), 1.0e-6f); }

        beginTest("Positive drift evolves playback");
        juce::AudioBuffer<float> driftBuffer(2, 4096);
        for (int i = 0; i < 4096; ++i) { const auto v = std::sin(i * 0.071f); driftBuffer.setSample(0, i, v); driftBuffer.setSample(1, i, v); }
        nachgluehen::LivingFreezeEngine driftEngine;
        driftEngine.prepare(48000.0, 4096);
        driftEngine.process(driftBuffer, false, 0.0f, 0.0f);
        driftBuffer.clear();
        driftEngine.process(driftBuffer, true, 0.85f, 1.0f);
        const auto first = driftBuffer.getSample(0, 3000);
        driftBuffer.clear();
        driftEngine.process(driftBuffer, true, 0.85f, 1.0f);
        expect(std::abs(driftBuffer.getSample(0, 3000) - first) > 1.0e-5f);

        beginTest("Mixing and transition continuity");
        nachgluehen::LivingFreezeEngine mixEngine;
        mixEngine.prepare(48000.0, 256);
        juce::AudioBuffer<float> mixBuffer(2, 256);
        for (int i = 0; i < 256; ++i) { mixBuffer.setSample(0, i, 0.2f); mixBuffer.setSample(1, i, 0.2f); }
        mixEngine.process(mixBuffer, false, 0.0f, 0.0f);
        for (int i = 0; i < 256; ++i) { mixBuffer.setSample(0, i, 0.8f); mixBuffer.setSample(1, i, 0.8f); }
        mixEngine.process(mixBuffer, true, 0.0f, 0.5f);
        for (int i = 1; i < 256; ++i)
            expect(std::abs(mixBuffer.getSample(0, i) - mixBuffer.getSample(0, i - 1)) < 0.5f);
        mixBuffer.clear();
        mixEngine.process(mixBuffer, true, 0.0f, 1.0f);
        expect(std::abs(mixBuffer.getSample(0, 128)) > 0.01f);

        beginTest("Zero drift is stable and outputs remain finite");
        nachgluehen::LivingFreezeEngine stableA, stableB;
        stableA.prepare(48000.0, 512); stableB.prepare(48000.0, 512);
        juce::AudioBuffer<float> stableSource(2, 512);
        for (int i = 0; i < 512; ++i) { const auto v = std::sin(i * 0.017f); stableSource.setSample(0, i, v); stableSource.setSample(1, i, v); }
        auto stableSourceCopy = stableSource;
        stableA.process(stableSource, false, 0.0f, 0.0f); stableB.process(stableSourceCopy, false, 0.0f, 0.0f);
        stableSource.clear(); stableSourceCopy.clear();
        stableA.process(stableSource, true, 0.0f, 1.0f); stableB.process(stableSourceCopy, true, 0.0f, 1.0f);
        for (int block = 0; block < 5; ++block)
        {
            for (int i = 0; i < 512; ++i) { expect(std::isfinite(stableSource.getSample(0, i))); expectWithinAbsoluteError(stableSource.getSample(0, i), stableSourceCopy.getSample(0, i), 1.0e-6f); }
            stableSource.clear(); stableSourceCopy.clear();
            stableA.process(stableSource, true, 0.0f, 1.0f); stableB.process(stableSourceCopy, true, 0.0f, 1.0f);
        }

        beginTest("Drift endpoints change identical frozen material");
        nachgluehen::LivingFreezeEngine zeroDrift, maximumDrift;
        zeroDrift.prepare(48000.0, 4096); maximumDrift.prepare(48000.0, 4096);
        zeroDrift.setSeed(7); maximumDrift.setSeed(7);
        juce::AudioBuffer<float> zeroInput(2, 4096), maximumInput(2, 4096);
        for (int i = 0; i < 4096; ++i)
        {
            const auto v = std::sin(i * 0.071f) + 0.2f * std::sin(i * 0.013f);
            zeroInput.setSample(0, i, v); zeroInput.setSample(1, i, v);
            maximumInput.setSample(0, i, v); maximumInput.setSample(1, i, v);
        }
        zeroDrift.process(zeroInput, false, 0.0f, 0.0f);
        maximumDrift.process(maximumInput, false, 0.0f, 0.0f);
        zeroInput.clear(); maximumInput.clear();
        zeroDrift.process(zeroInput, true, 0.0f, 1.0f);
        maximumDrift.process(maximumInput, true, 1.0f, 1.0f);
        double difference = 0.0;
        for (int block = 0; block < 3; ++block)
        {
            for (int i = 0; i < 4096; ++i)
            {
                expect(std::isfinite(zeroInput.getSample(0, i)));
                expect(std::isfinite(maximumInput.getSample(0, i)));
                difference += std::abs(zeroInput.getSample(0, i) - maximumInput.getSample(0, i));
            }
            zeroInput.clear(); maximumInput.clear();
            zeroDrift.process(zeroInput, true, 0.0f, 1.0f);
            maximumDrift.process(maximumInput, true, 1.0f, 1.0f);
        }
        expectGreaterThan(difference, 0.001);

        beginTest("Higher Drift produces a stronger seeded change than low Drift");
        nachgluehen::LivingFreezeEngine zeroForComparison, lowDrift, highDrift;
        zeroForComparison.prepare(48000.0, 4096);
        lowDrift.prepare(48000.0, 4096); highDrift.prepare(48000.0, 4096);
        zeroForComparison.setSeed(91);
        lowDrift.setSeed(91); highDrift.setSeed(91);
        juce::AudioBuffer<float> lowBuffer(2, 4096), highBuffer(2, 4096), zeroBuffer(2, 4096);
        for (int i = 0; i < 4096; ++i)
        {
            const auto v = std::sin(i * 0.071f) + 0.2f * std::sin(i * 0.013f);
            lowBuffer.setSample(0, i, v); lowBuffer.setSample(1, i, v);
            highBuffer.setSample(0, i, v); highBuffer.setSample(1, i, v);
            zeroBuffer.setSample(0, i, v); zeroBuffer.setSample(1, i, v);
        }
        zeroForComparison.process(zeroBuffer, false, 0.0f, 0.0f);
        lowDrift.process(lowBuffer, false, 0.0f, 0.0f);
        highDrift.process(highBuffer, false, 0.0f, 0.0f);
        zeroBuffer.clear(); lowBuffer.clear(); highBuffer.clear();
        zeroForComparison.process(zeroBuffer, true, 0.0f, 1.0f);
        lowDrift.process(lowBuffer, true, 0.15f, 1.0f);
        highDrift.process(highBuffer, true, 1.0f, 1.0f);
        double lowDifference = 0.0, highDifference = 0.0;
        for (int i = 0; i < 4096; ++i)
        {
            lowDifference += std::abs(lowBuffer.getSample(0, i) - zeroBuffer.getSample(0, i));
            highDifference += std::abs(highBuffer.getSample(0, i) - zeroBuffer.getSample(0, i));
        }
        expectGreaterThan(lowDifference, 0.0001);
        expectGreaterThan(highDifference, 0.001);
    }
};
LivingFreezeTests livingFreezeTests;
}

int main()
{
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure(false);
    runner.runTestsInCategory("Nachgluehen", 1);
    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        if (const auto* result = runner.getResult(i))
        {
            for (const auto& message : result->messages)
                std::cerr << result->subcategoryName << ": " << message << "\n";
            failures += result->failures;
        }
    return failures == 0 ? 0 : 1;
}
