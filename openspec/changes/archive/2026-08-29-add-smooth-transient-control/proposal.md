## Why

Transient-rich captured material can repeatedly expose its original attacks as texture voices restart at different safe positions. Those attacks are musically useful for rhythmic and granular results, but the plugin currently offers no musical control to turn the same capture into a substantially softer, pad-like texture.

## What Changes

- Add an automatable, persisted `Smooth` percentage parameter (0–100%, default 50%) that controls the retained transient character of the wet Living Freeze texture.
- Shape texture-voice windowing, restart audibility, texture-gain-compensation response, and—where needed—a lightweight wet-only transient smoother from the single Smooth control.
- Preserve safe read/start regions, zero-boundary voice envelopes, click-free transitions, and independent Drift movement.
- Add a `SMOOTH` rotary control using the existing knob styling while keeping the editor fixed-size and all rotary controls visually equal.
- Add deterministic DSP and parameter/state regression coverage for Smooth behavior, transient reduction, gain stability, click safety, and Drift independence.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `living-freeze`: Add Smooth-controlled transient, window, restart, and gain-response behavior to the wet texture while preserving safe continuous playback.
- `plugin-parameters-and-state`: Expose Smooth as a normalized percentage parameter and include it in automation and serialized state.
- `nachgluehen-ui`: Present Smooth as an equal-sized rotary control in the fixed custom editor.

## Impact

- Affected DSP: `LivingFreezeEngine` texture rendering, voice envelope/restart handling, wet gain compensation, and parameter smoothing.
- Affected host contract and UI: parameter layout, processor wiring/state restoration, editor controls/layout, and JUCE unit tests.
- No new external dependencies or additional visible detail parameters.
