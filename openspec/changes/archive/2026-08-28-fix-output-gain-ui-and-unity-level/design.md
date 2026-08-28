## Context

The current Output Gain parameter uses a linear dB range from its mute representation to +12 dB, so the 0 dB value is not at the rotary midpoint. The fixed editor allocates the Output control and its labels from the same column, reducing its knob bounds and risking an empty `CLIP` bounds rectangle. The existing texture engine already uses bounded, smoothed energy compensation; see proposal.md for the motivation and delta specs for observable requirements.

## Goals / Non-Goals

**Goals:**

- Make the fixed UI visually balanced and the clip status independently renderable.
- Make normalized 0.5 a precise unity-gain anchor while retaining mute and +12 dB endpoints.
- Establish a stable wet texture level without changing Dry/Wet endpoint semantics or using final Output Gain as internal compensation.
- Preserve real-time-safe smoothing, metering, and existing click-safe texture behavior.

**Non-Goals:**

- Adding a limiter, compressor, loudness normalizer, or new exposed gain parameter.
- Changing plugin window size, existing parameter identifiers, or the Output Gain dB endpoints.
- Replacing the existing clip threshold or clip-hold architecture.

## Decisions

### Piecewise centred Output Gain normalization

Use an invertible piecewise mapping: the lower normalized half maps from mute to 0 dB, while the upper half maps linearly from 0 to +12 dB. This gives 0 dB an exact centre position and retains meaningful fine adjustment around unity. A linear -100 to +12 dB range is rejected because it places unity far from centre; a new parameter is rejected because it would break the existing host-facing control contract.

### Equal rotary bounds with independent Output metadata

Allocate equal control rectangles for all three rotary sliders. Position the Output label and `CLIP` label in separately reserved rows or columns, with nonzero explicit bounds. The timer continues to poll the lock-free processor hold state, but visibility does not depend on remaining slider geometry.

### Conservative texture-level calibration

Measure the current window/voice compensation behavior with deterministic test material, then adjust only the bounded and smoothed internal compensation or related static mix factors required to prevent systematic wet-level loss. Preserve envelope shape and bounded peaks. A rapid automatic level follower is rejected because it can pump and alter the texture character.

### Final gain and clipping remain downstream

Keep the signal order as texture/Dry-Wet, smoothed Output Gain, clip detection, and plugin output. 0 dB remains a linear factor of 1.0, so any wet-level improvement is attributable to the texture engine rather than a hidden output boost.

## Risks / Trade-offs

- [Existing host automation may use normalized positions] → Preserve the parameter identifier and dB state values, and cover state restore plus normalized endpoint/midpoint conversion in tests.
- [A compensation increase raises peaks] → Keep compensation bounded and slow, retain final post-gain clipping detection, and add finite/level regression checks.
- [Dense fixed layout can overlap controls] → Use explicit equal slider and metadata rectangles and cover their geometry with editor-level checks where practical.

## Migration Plan

Existing saved parameter states retain their stored Output Gain dB values. The host-facing identifier and dB range remain unchanged; rollback restores the previous normalization and texture compensation implementation without requiring project migration.
