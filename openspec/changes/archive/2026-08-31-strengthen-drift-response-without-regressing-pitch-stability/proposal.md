## Why

The shared global-pitch Drift architecture has removed the previous transient-like pitch artifacts and made frozen textures coherent, but its current response is too restrained through the middle and upper portion of the control. Drift needs a stronger, more musically useful range of pitch, position, and spatial evolution without sacrificing stability, level, or the sense of one shared texture.

## What Changes

- Increase the bounded shared global pitch range and its gradual random-walk target movement so high Drift produces meaningful controlled pitch wandering.
- Make the nonlinear pitch response begin earlier in the control range while keeping low Drift subtle and preserving slow dedicated pitch smoothing.
- Increase bounded position movement, with an optional modest adjustment to the movement response curve, so medium and high Drift provide more texture and spatial evolution without relying on independent voice detuning.
- Keep local voice pitch variation deliberately small and subordinate to the shared global pitch component.
- Add deterministic JUCE regression coverage for zero Drift, pitch and position safety bounds, mid-control response, smoothing, long renders, and gain stability across the Drift range.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `living-freeze`: Strengthen the bounded Drift response while preserving globally coherent, smooth, safe, click-free frozen-texture playback and controlled wet level.

## Impact

- Affected DSP: `LivingFreezeEngine` drift constants, response mappings, target generation, and existing diagnostics as needed for deterministic tests.
- Affected tests: JUCE unit tests for living-freeze behavior and long-render safety/gain regressions.
- No parameter, UI, state-format, external API, or dependency changes are expected.
