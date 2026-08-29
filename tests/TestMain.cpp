#include "LivingFreezeEngine.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>
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
        auto* smoothParameter = processor.parameters.getParameter(nachgluehen::parameterIds::smooth);
        auto* dryWetParameter = processor.parameters.getParameter(nachgluehen::parameterIds::dryWet);
        auto* outputGainParameter = processor.parameters.getParameter(nachgluehen::parameterIds::outputGain);
        expect(freezeParameter != nullptr && driftParameter != nullptr && smoothParameter != nullptr && dryWetParameter != nullptr && outputGainParameter != nullptr);
        expectWithinAbsoluteError(freezeParameter->getDefaultValue(), 0.0f, 1.0e-6f);
        expectWithinAbsoluteError(driftParameter->getDefaultValue(), 0.20f, 1.0e-6f);
        expectWithinAbsoluteError(smoothParameter->getDefaultValue(), 0.50f, 1.0e-6f);
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
        expectWithinAbsoluteError(smoothParameter->getNormalisableRange().start, 0.0f, 1.0e-6f);
        expectWithinAbsoluteError(smoothParameter->getNormalisableRange().end, 1.0f, 1.0e-6f);
        expectEquals(smoothParameter->getText(0.5f, 8), juce::String("50"));
        expectWithinAbsoluteError(smoothParameter->getValueForText("100"), 1.0f, 1.0e-6f);
        processor.parameters.getParameter(nachgluehen::parameterIds::freeze)->setValueNotifyingHost(1.0f);
        processor.parameters.getParameter(nachgluehen::parameterIds::drift)->setValueNotifyingHost(0.73f);
        processor.parameters.getParameter(nachgluehen::parameterIds::smooth)->setValueNotifyingHost(0.91f);
        processor.parameters.getParameter(nachgluehen::parameterIds::dryWet)->setValueNotifyingHost(0.11f);
        outputGainParameter->setValueNotifyingHost(outputGainParameter->getNormalisableRange().convertTo0to1(12.0f));
        juce::MemoryBlock state;
        processor.getStateInformation(state);
        NachgluehenAudioProcessor restored;
        restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        expectWithinAbsoluteError(restored.parameters.getRawParameterValue(nachgluehen::parameterIds::drift)->load(), 0.73f, 0.002f);
        expectWithinAbsoluteError(restored.parameters.getRawParameterValue(nachgluehen::parameterIds::smooth)->load(), 0.91f, 0.002f);
        expectWithinAbsoluteError(restored.parameters.getRawParameterValue(nachgluehen::parameterIds::dryWet)->load(), 0.11f, 0.002f);
        expect(restored.parameters.getRawParameterValue(nachgluehen::parameterIds::freeze)->load() >= 0.5f);
        expectWithinAbsoluteError(restored.parameters.getRawParameterValue(nachgluehen::parameterIds::outputGain)->load(), 12.0f, 0.11f);

        beginTest("Smooth editor control uses the shared equal-size rotary layout");
        juce::ScopedJuceInitialiser_GUI gui;
        NachgluehenAudioProcessor editorProcessor;
        std::unique_ptr<juce::AudioProcessorEditor> editor(editorProcessor.createEditor());
        const auto* driftControl = dynamic_cast<const juce::Slider*>(editor->findChildWithID("drift-control"));
        const auto* smoothControl = dynamic_cast<const juce::Slider*>(editor->findChildWithID("smooth-control"));
        const auto* dryWetControl = dynamic_cast<const juce::Slider*>(editor->findChildWithID("dry-wet-control"));
        const auto* outputControl = dynamic_cast<const juce::Slider*>(editor->findChildWithID("output-gain-control"));
        expect(driftControl != nullptr && smoothControl != nullptr && dryWetControl != nullptr && outputControl != nullptr);
        if (driftControl != nullptr && smoothControl != nullptr && dryWetControl != nullptr && outputControl != nullptr)
        {
            expectEquals(driftControl->getBounds().getWidth(), smoothControl->getBounds().getWidth());
            expectEquals(smoothControl->getBounds().getWidth(), dryWetControl->getBounds().getWidth());
            expectEquals(dryWetControl->getBounds().getWidth(), outputControl->getBounds().getWidth());
            expectEquals(driftControl->getBounds().getHeight(), smoothControl->getBounds().getHeight());
            expectEquals(smoothControl->getBounds().getHeight(), dryWetControl->getBounds().getHeight());
            expectEquals(dryWetControl->getBounds().getHeight(), outputControl->getBounds().getHeight());
            expectWithinAbsoluteError(static_cast<float>(smoothControl->getValue()), 0.5f, 1.0e-6f);
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

        beginTest("Smooth window endpoints remain continuous");
        for (const auto smooth : { 0.0f, 0.5f, 1.0f })
        {
            expectWithinAbsoluteError(nachgluehen::LivingFreezeEngine::windowValue(0.0, smooth), 0.0f, 1.0e-6f);
            expectWithinAbsoluteError(nachgluehen::LivingFreezeEngine::windowValue(1.0, smooth), 0.0f, 1.0e-6f);
            float previous = 0.0f;
            for (int i = 0; i <= 200; ++i)
            {
                const auto value = nachgluehen::LivingFreezeEngine::windowValue(i / 200.0, smooth);
                expect(value >= 0.0f && std::isfinite(value));
                expectLessThan(std::abs(value - previous), 0.03f);
                previous = value;
            }
        }

        beginTest("Smooth reduces transient energy while retaining Drift movement");
        const auto renderTransientTexture = [](float smooth, float drift, float& maximumStep)
        {
            nachgluehen::LivingFreezeEngine smoothEngine;
            smoothEngine.prepare(48000.0, 256);
            smoothEngine.setSeed(1337);
            juce::AudioBuffer<float> smoothBuffer(2, 256);
            for (int block = 0; block < 128; ++block)
            {
                for (int sample = 0; sample < smoothBuffer.getNumSamples(); ++sample)
                {
                    const auto index = block * smoothBuffer.getNumSamples() + sample;
                    const auto value = 0.16f * std::sin(index * 0.041f) + (index % 1200 == 0 ? 1.0f : 0.0f);
                    smoothBuffer.setSample(0, sample, value);
                    smoothBuffer.setSample(1, sample, value);
                }
                smoothEngine.process(smoothBuffer, false, drift, 0.0f, smooth);
            }
            double attackEnergy = 0.0;
            float previous = 0.0f;
            maximumStep = 0.0f;
            for (int block = 0; block < 180; ++block)
            {
                smoothBuffer.clear();
                smoothEngine.process(smoothBuffer, true, drift, 1.0f, smooth);
                for (int sample = 0; sample < smoothBuffer.getNumSamples(); ++sample)
                {
                    const auto value = smoothBuffer.getSample(0, sample);
                    attackEnergy += juce::jmax(0.0f, std::abs(value) - std::abs(previous));
                    maximumStep = juce::jmax(maximumStep, std::abs(value - previous));
                    previous = value;
                }
            }
            return attackEnergy;
        };
        float smoothZeroStep = 0.0f, smoothMaximumStep = 0.0f, smoothDriftStep = 0.0f;
        const auto zeroAttackEnergy = renderTransientTexture(0.0f, 0.0f, smoothZeroStep);
        const auto maximumAttackEnergy = renderTransientTexture(1.0f, 0.0f, smoothMaximumStep);
        const auto driftAttackEnergy = renderTransientTexture(1.0f, 1.0f, smoothDriftStep);
        expect(maximumAttackEnergy < zeroAttackEnergy * 0.85,
               "zero=" + juce::String(zeroAttackEnergy) + " maximum=" + juce::String(maximumAttackEnergy));
        expectGreaterThan(driftAttackEnergy, 0.001, "drift attack energy=" + juce::String(driftAttackEnergy));
        expectLessThan(smoothMaximumStep, 0.75f, "maximum Smooth step=" + juce::String(smoothMaximumStep));
        expectLessThan(smoothDriftStep, 0.75f, "maximum Smooth/Drift step=" + juce::String(smoothDriftStep));

        beginTest("Smooth automation remains finite and click-safe");
        nachgluehen::LivingFreezeEngine smoothAutomationEngine;
        smoothAutomationEngine.prepare(48000.0, 1);
        juce::AudioBuffer<float> smoothAutomationBuffer(2, 1);
        for (int sample = 0; sample < 4096; ++sample)
        {
            const auto value = std::sin(sample * 0.071f) + (sample % 311 == 0 ? 0.8f : 0.0f);
            smoothAutomationBuffer.setSample(0, 0, value);
            smoothAutomationBuffer.setSample(1, 0, value);
            smoothAutomationEngine.process(smoothAutomationBuffer, false, 0.0f, 0.0f, 0.0f);
        }
        float previousSmoothOutput = 0.0f;
        float maximumSmoothAutomationStep = 0.0f;
        for (int sample = 0; sample < 4096; ++sample)
        {
            smoothAutomationBuffer.clear();
            const auto smooth = sample < 1024 ? 0.0f : (sample < 2048 ? 1.0f : 0.35f);
            smoothAutomationEngine.process(smoothAutomationBuffer, true, sample % 2 == 0 ? 1.0f : 0.6f, 1.0f, smooth);
            const auto output = smoothAutomationBuffer.getSample(0, 0);
            expect(std::isfinite(output));
            maximumSmoothAutomationStep = juce::jmax(maximumSmoothAutomationStep, std::abs(output - previousSmoothOutput));
            previousSmoothOutput = output;
        }
        expectLessThan(maximumSmoothAutomationStep, 0.75f);

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
            expect(energyEngine.getTextureGainCompensation() >= 0.14f - 1.0e-4f);
            expect(energyEngine.getTextureGainCompensation() <= 0.60f + 1.0e-4f);
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
            expectLessThan(peak, 1.25f);
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
