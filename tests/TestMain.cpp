#include "LivingFreezeEngine.h"
#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>

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
            failures += result->failures;
    return failures == 0 ? 0 : 1;
}
