## Context

The repository contains a JUCE/CMake VST3 effect with a Living-Freeze engine, host parameters, custom editor, and JUCE unit tests. The revised MVP requires ASCII-safe visible text, an explicit normalized Drift mapping, reliable parameter-to-DSP wiring, and a fixed editor size. The frozen audio remains transient; only visible parameters are project state.

## Goals / Non-Goals

**Goals:**

- Keep the existing real-time-safe Living-Freeze architecture and custom atmospheric LookAndFeel.
- Present the visible plugin name as Nachgluehen everywhere.
- Map the full Drift UI range 0-100% to normalized DSP values 0.0-1.0 and verify the connection in tests.
- Ensure Drift 0% is intentionally stable and Drift 100% reaches the maximum configured non-periodic modulation intensity.
- Use a fixed editor size where all knobs, labels, and the Freeze toggle are completely visible.
- Verify Drift mapping, endpoints, measurable DSP response, state, and finite output with JUCE Unit Tests.

**Non-Goals:**

- Reverb, delay, tempo synchronization, MIDI, presets, overdubbing, reverse playback, distortion, bit reduction, or persisted frozen audio.
- Responsive resizing, scalable layout behavior, and automated subjective assessment of UI or sound.

## Decisions

### ASCII-safe visible naming

Use Nachgluehen for product metadata, processor display name, editor title, labels, and other visible text. Keep source and file identifiers ASCII-safe as well. This avoids platform-dependent rendering of the umlaut.

Alternative considered: retaining the umlaut in product metadata and relying on UTF-8 rendering. The ASCII spelling is preferred because the current host/UI path does not render it reliably.

### Explicit normalized Drift mapping

Represent the host parameter as a normalized JUCE parameter with range 0.0-1.0 and expose it in the editor as a percentage by using the parameter's normalisable range and a percent suffix. A UI value of 50% must resolve to normalized 0.5, while 0% and 100% resolve to 0.0 and 1.0. Pass the atomic normalized value directly to the engine after clamping; do not apply a second percentage division or integer conversion.

Alternative considered: storing 0-100 internally. Normalized storage matches JUCE host automation and prevents the observed 0-1% display/wiring error.

### Audible, non-periodic Drift response

Keep the seedable smoothed pseudo-random target/random-walk approach. Drift 0% disables intentional movement. Positive values influence bounded read speed/position, window or crossfade behavior, and small stereo offsets. Drift 100% uses the maximum configured intensity. Unit tests compare identical frozen material at Drift 0% and positive/end-point values to prove the parameter reaches the DSP and changes output.

Alternative considered: a cyclic LFO. It is explicitly rejected because it would not satisfy the non-periodic requirement.

### Fixed editor geometry

Use one fixed editor size and fixed control bounds chosen so the Freeze toggle, Drift knob, Dry/Wet knob, labels, and values are completely visible. Retain the custom LookAndFeel and atmospheric drawing, but remove responsive resize logic and resize-specific acceptance criteria.

Alternative considered: proportional bounds and resize limits. They are unnecessary for this MVP and made it harder to guarantee consistent control visibility.

### Deterministic tests

Use only the JUCE Unit Test Framework. Add tests for normalized Drift mapping, 0% and 100% endpoint handling, and measurable output divergence between Drift 0% and Drift > 0% for identical frozen input. Keep Ableton Live listening tests manual.

## Risks / Trade-offs

- [Risk] Hosts may display normalized parameter values differently. -> Use explicit parameter text conversion and test the normalized-to-percent contract.
- [Risk] Maximum Drift may become too unstable musically. -> Bound all modulation targets and validate finite output; judge musical intensity manually in Ableton Live.
- [Risk] A fixed editor may not fit every host preference. -> Choose a conservative fixed size and document that resizing is outside the MVP.
- [Risk] ASCII naming differs from the original umlaut spelling. -> Apply the same spelling to metadata and every visible string so hosts and users see one consistent name.

## Migration Plan

Update the existing plugin metadata, visible text, parameter mapping, editor geometry, DSP wiring, and tests. Rebuild the VST3 and run the JUCE test target. Perform final subjective Drift and UI checks manually in Ableton Live. Existing project parameter IDs remain stable; transient frozen audio remains unsaved.

## Open Questions

None. Exact fixed dimensions and the precise bounded Drift shaping may be tuned during implementation without changing the revised behavior contract.
