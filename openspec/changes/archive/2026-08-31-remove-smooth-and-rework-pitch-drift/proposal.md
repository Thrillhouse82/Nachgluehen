## Why

The Smooth control fails to provide useful musical control: it primarily reduces energy and does not reliably address the transient-like events introduced by pitch drift. The stable, continuous Drift=0 texture should be retained while Drift is made coherent and tape-like rather than a collection of independently detuned voices.

## What Changes

- **BREAKING** Remove the visible and host-exposed Smooth parameter, its state serialization, editor control, processor/engine API, and related tests. The visible parameter set becomes Freeze, Drift, Dry/Wet, and Output Gain.
- Remove frozen-transient analysis, source and post-texture transient attenuation, and Smooth-dependent gain-compensation behavior; retain a single fixed click-safe baseline window and stable gain compensation.
- Rework pitch drift around a shared, slow, non-periodic global playback-speed random walk, with only small slowly changing per-voice pitch factors.
- Preserve the existing capture duration, overlapping texture voices, safe read/start regions, position/stereo drift, and click-safe restart behavior.
- Add regression and behavioral coverage for zero-drift pitch identity, global pitch coherence, local pitch bounds, smoothing/restart continuity, level stability, and long renders.

## Capabilities

### New Capabilities

- None.

### Modified Capabilities

- `living-freeze`: Remove Smooth-dependent texture behavior and require coherent shared pitch drift while preserving a stable zero-drift overlapping texture.
- `nachgluehen-ui`: Remove Smooth from the fixed custom editor control set and layout.
- `plugin-parameters-and-state`: Remove Smooth from the exposed, automatable, and serialized parameter set.

## Impact

- Affected code: parameter definitions and state, processor-to-engine integration, Living Freeze engine DSP, custom editor layout/attachments, and JUCE unit tests.
- Host compatibility: previously saved Smooth values will no longer correspond to an exposed parameter after the breaking removal.
- No new external dependencies or real-time-unsafe processing are introduced.
