## Context

The current Living Freeze engine reads a captured stereo fragment through eight overlapping, sine-windowed voices. Safe regions, near-zero restart boundaries, gradual Drift movement, and energy-aware gain compensation already prevent mechanical clicks, but they intentionally leave captured attacks audible. See `proposal.md` for the motivation and the delta specs for required behavior.

## Goals / Non-Goals

**Goals:**

- Add one smoothed, host-facing `Smooth` control that gives a repeatable continuum from the current transient-forward behavior to a softer texture.
- Keep all DSP work in the prepared, allocation-free callback path and retain current safe-read/restart guarantees.
- Keep Smooth orthogonal to Drift and preserve the existing Output Gain stage behavior.

**Non-Goals:**

- Building a compressor, limiter, spectral process, or a user-visible collection of advanced texture controls.
- Changing capture duration, safe-region policy, Drift mapping, dry signal, editor resize policy, or Output Gain's mapping.
- Forcing identical RMS between Smooth extremes or eliminating every musical transient.

## Decisions

### One parameter fans out to complementary wet-only controls

The engine receives a normalized Smooth value alongside Freeze, Drift, and Dry/Wet and smooths it sample-by-sample. The current value derives three correlated internal controls: a window-shape exponent that retains exact zero endpoints, a restart exposure/fade policy that only changes state at already-safe near-zero boundaries, and a gain-compensation time constant. The compensation time grows nonlinearly from approximately 10 ms at Smooth 0 to approximately 250 ms at Smooth 1, giving a clear but bounded difference without turning gain adaptation into a multi-second recovery.

The primary transient reducer analyses the captured fragment once at Freeze activation and stores a preallocated, decaying marker for attack-rich regions. Each voice applies a Smooth-dependent attenuation from that marker while reading the frozen source, so the reduction remains effective even when several voices overlap in the wet mix. A lightweight envelope-following wet-output attenuator provides additional protection against short residual attacks. Both layers use only prepared state and a Smooth-dependent blend rather than broadband low-pass filtering. This preserves brightness and Drift movement more reliably than a simple low-pass filter; a full dynamics processor is rejected as excessive scope and complexity.

### Preserve baseline at Smooth zero and safe sine boundaries

Smooth 0 will map to the present sine window and fastest existing-style compensation response as closely as feasible. Higher values will shape the sine-derived envelope so it remains continuous, non-negative, and exactly zero at phase 0 and 1, while spending relatively more audible time in gentler fade regions. Voice positions will never be changed outside the existing restart condition or safety limits.

Changing raw window length or allowing an early restart was considered, but rejected because it risks exposing new buffer content during an audible envelope and weakening established click safety.

### Integrate through APVTS and retain a compact four-knob layout

`smooth` will be a 0.0–1.0 percent `AudioParameterFloat` with default 0.5, supplied from the processor to the engine every block. APVTS makes it automatable and serializable with the existing contract. The editor will add one attachment and reorganize its fixed content area into four equal rotary columns while retaining the existing custom look-and-feel and separately bounded clip indicator.

### Deterministic, behavior-oriented regression measurements

Tests will render seeded engines with matched transient-rich captures at the endpoints, measure short-window peaks/energy deltas, and separately assert finite samples, bounded compensation, continuous window endpoints, and no excessive sample steps across Smooth/Drift combinations. Parameter tests will cover range/default, text representation, APVTS state restore, and host-style parameter updates. A high-Drift/high-Smooth test will compare its evolution to a zero-Drift reference to prove independence.

## Risks / Trade-offs

- [High Smooth becomes dull or static] → Apply any transient reduction only to transient envelopes, keep it wet-only, and verify high-Smooth Drift remains measurably active.
- [Window reshaping reduces average level] → Keep bounded energy-aware compensation, slow its high-Smooth response, and test audibility without enforcing RMS equality.
- [Endpoint tests pass but automation audibly steps] → Smooth the parameter internally and include rapid automation in continuity regression renders.
- [New gain timing breaks prior level tests] → Preserve compensation limits and update/add tests around controlled level rather than constant gain.

## Migration Plan

Existing projects load with Smooth at its defined default when no stored value is present; new states serialize it through APVTS. The change has no external data migration and can be rolled back by removing the parameter and its UI/DSP wiring, with old project state continuing to load because unknown parameter data is ignored.
