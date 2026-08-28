## Why

The Output section does not currently present a dependable neutral gain control: its knob is visually smaller than the other controls, its clip state can be invisible, and 0 dB is not centred. In addition, Output Gain must remain a unity trim rather than compensate for a systematically weak texture level.

## What Changes

- Give Output the same rotary-control size and visual treatment as Drift and Dry/Wet, while assigning the `CLIP` label an explicit independent layout area.
- Keep the existing post-output clip threshold and hold semantics, but require the status to be reliably visible whenever active.
- Use an asymmetric, invertible Output Gain parameter mapping: mute at normalized 0.0, 0 dB at 0.5, and +12 dB at 1.0.
- Preserve 0 dB as exact final unity gain and improve the internal texture gain strategy only where needed to avoid systematic low wet-level output.
- Add focused mapping, output-level, clip-hold, and texture-level regression coverage.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `nachgluehen-ui`: Require equal-sized rotary controls and a reliably visible Output clip status layout.
- `plugin-parameters-and-state`: Define the centred asymmetric Output Gain mapping and its round-trip, text, automation, state, and unity behavior.
- `living-freeze`: Require a musically useful wet texture level without using Output Gain as internal level compensation.

## Impact

- Affects Output parameter normalization/display, final gain processing, editor layout, and texture gain compensation.
- Existing hosts retain the same parameter identifier and dB endpoints; saved normalized Output Gain values are interpreted by the newly specified centred mapping.
- Requires JUCE unit-test updates; adds no dependencies and does not add a limiter, compressor, or visible internal gain control.
