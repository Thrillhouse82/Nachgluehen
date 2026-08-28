## Why

The frozen texture can be intentionally quieter than source material, and users currently have no direct way to match its final output level to a session. A dedicated output trim and a visible overload warning make that adjustment predictable without changing the texture character.

## What Changes

- Add a host-automatable Output Gain control after Dry/Wet mixing, defaulting to 0 dB.
- Support a mute endpoint and a bounded positive boost; do not expose unbounded positive gain.
- Smooth Output Gain changes to avoid parameter zipper noise.
- Add a small red `CLIP` indicator that monitors the final plugin output, activates just below full scale, and remains visible briefly after a detected peak.
- Persist the Output Gain setting with the plugin state.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `plugin-parameters-and-state`: Add the Output Gain parameter, its host automation behavior, processing position, range, and state persistence.
- `nachgluehen-ui`: Add the Output Gain rotary control and a compact red `CLIP` status indicator to the fixed editor layout.

## Impact

- Affects parameter definitions, processor output staging, editor controls and state display.
- Requires real-time-safe metering communication from the audio processor to the editor.
- No new dependencies and no changes to the existing Freeze, Drift, or Dry/Wet parameter meanings.
