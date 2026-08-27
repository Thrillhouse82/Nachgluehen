#include "LivingFreezeEngine.h"
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
        auto* dryWetParameter = processor.parameters.getParameter(nachgluehen::parameterIds::dryWet);
        expect(freezeParameter != nullptr && driftParameter != nullptr && dryWetParameter != nullptr);
        expectWithinAbsoluteError(freezeParameter->getDefaultValue(), 0.0f, 1.0e-6f);
        expectWithinAbsoluteError(driftParameter->getDefaultValue(), 0.20f, 1.0e-6f);
        expectWithinAbsoluteError(dryWetParameter->getDefaultValue(), 0.50f, 1.0e-6f);
        expectWithinAbsoluteError(driftParameter->getNormalisableRange().start, 0.0f, 1.0e-6f);
        expectWithinAbsoluteError(driftParameter->getNormalisableRange().end, 1.0f, 1.0e-6f);
        expectEquals(driftParameter->getText(0.5f, 8), juce::String("50"));
        expectWithinAbsoluteError(driftParameter->getValueForText("50"), 0.5f, 1.0e-6f);
        processor.parameters.getParameter(nachgluehen::parameterIds::freeze)->setValueNotifyingHost(1.0f);
        processor.parameters.getParameter(nachgluehen::parameterIds::drift)->setValueNotifyingHost(0.73f);
        processor.parameters.getParameter(nachgluehen::parameterIds::dryWet)->setValueNotifyingHost(0.11f);
        juce::MemoryBlock state;
        processor.getStateInformation(state);
        NachgluehenAudioProcessor restored;
        restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        expectWithinAbsoluteError(restored.parameters.getRawParameterValue(nachgluehen::parameterIds::drift)->load(), 0.73f, 0.002f);
        expectWithinAbsoluteError(restored.parameters.getRawParameterValue(nachgluehen::parameterIds::dryWet)->load(), 0.11f, 0.002f);
        expect(restored.parameters.getRawParameterValue(nachgluehen::parameterIds::freeze)->load() >= 0.5f);

        beginTest("Defaults and dry passthrough");
        nachgluehen::LivingFreezeEngine engine;
        engine.prepare(48000.0, 128);
        juce::AudioBuffer<float> buffer(2, 128);
        buffer.clear();
        for (int i = 0; i < 128; ++i) { buffer.setSample(0, i, 0.25f); buffer.setSample(1, i, -0.25f); }
        engine.process(buffer, false, 0.2f, 0.0f);
        expectWithinAbsoluteError(buffer.getSample(0, 64), 0.25f, 1.0e-6f);
        expectWithinAbsoluteError(buffer.getSample(1, 64), -0.25f, 1.0e-6f);

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
        expectWithinAbsoluteError(captureBuffer.getSample(0, 127), 0.35f, 0.04f);
        expectWithinAbsoluteError(captureBuffer.getSample(1, 127), 0.35f, 0.04f);

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
            if (block == 1)
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
        expectLessThan(gainMaximum, 1.0f);

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
        expectGreaterThan(highDifference, lowDifference);
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
