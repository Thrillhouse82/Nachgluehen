## Why

Nachgluehen currently captures too much recent history when Freeze is activated, which can make the frozen texture alternate between sound and silence. Its Drift modulation is also too small to create the intended audible, slowly living texture. This change tightens capture to a short pre-trigger window and strengthens the existing non-periodic movement while preserving the MVP parameter and state contract.

## What Changes

- Capture a fixed, implementation-selected window of approximately 500–750 ms immediately before Freeze is triggered.
- Keep the captured material independent from subsequent live input once Freeze is active.
- Play the captured window as a continuous, click-free texture using suitable boundary smoothing such as overlap/crossfade processing.
- Preserve stable playback with no intentional movement at Drift 0%.
- Increase read-position and playback-speed modulation to musically useful ranges as Drift increases, with smooth, bounded, pseudo-random/non-periodic evolution.
- Add deterministic JUCE Unit Test coverage for capture placement and length, frozen-buffer immutability, continuity, finite output, and Drift behavior across the full range.
- Keep Freeze, Drift, and Dry/Wet parameters, host automation, plugin state, and MVP scope unchanged.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `living-freeze`: Change capture to a bounded immediately-preceding window, require continuous crossfaded/overlapped playback, and strengthen measurable smooth non-periodic Drift behavior.

## Impact

- Affected DSP code in the living-freeze/audio processor implementation and its preallocated runtime state.
- Affected JUCE Unit Tests for the living-freeze processor behavior.
- No new user-facing parameters, host-facing APIs, dependencies, or plugin-state fields.
