## Why

Musical freeze effects often require a chain of looping, modulation, resampling, and spatial effects, while a simple loop quickly sounds static and artificial. Nachgluehen introduces a focused, self-contained VST3 effect that captures a recent live moment and keeps it evolving as an atmospheric texture.

## What Changes

- Create a JUCE/CMake VST3 audio-effect plugin named Nachgluehen. This ASCII spelling SHALL be used in all visible plugin and UI text.
- Provide a real-time stereo Living-Freeze processor that continuously retains recent input while unfrozen and captures a short recent fragment when Freeze is enabled.
- Play the captured fragment indefinitely with click-free transitions and non-periodic, smoothed drift.
- Expose host-automatable and persistent Freeze, Drift, and Dry/Wet parameters. Drift SHALL map the complete UI range 0-100% to the normalized DSP range 0.0-1.0.
- Add a minimal fixed-size custom atmospheric interface with a dedicated Freeze toggle and rotary controls. Responsive resizing is outside the MVP.
- Add deterministic JUCE Unit Test Framework coverage for parameter behavior, state serialization, freeze capture/playback, Drift mapping and endpoints, measurable drift response, blending, continuity, and finite output.
- Keep reverb, delay, looping/overdub, tempo sync, distortion, bit reduction, presets, and MIDI processing outside the MVP.
- Keep subjective Drift, UI, and sound evaluation as manual Ableton Live validation.

## Capabilities

### New Capabilities

- `living-freeze`: Capture recent stereo input and produce an indefinitely sustaining, click-free, non-periodically drifting frozen texture with Dry/Wet mixing.
- `plugin-parameters-and-state`: Define the Freeze, Drift, and Dry/Wet parameter contract, normalized mapping, host automation behavior, and project-state persistence.
- `nachgluehen-ui`: Define the custom, fixed-size atmospheric plugin interface and its visible controls.

### Modified Capabilities

- None.

## Impact

- Adds a JUCE/CMake C++ project structure, plugin target, DSP engine, editor, and dedicated unit-test target.
- Adds ASCII-safe visible plugin metadata and UI text, normalized Drift mapping, parameter state serialization, and real-time audio processing.
- No existing application or capability is modified; the frozen audio buffer is intentionally not persisted in the MVP.
