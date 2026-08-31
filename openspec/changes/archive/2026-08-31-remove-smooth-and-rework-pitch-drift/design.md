## Context

See `proposal.md` for motivation. The existing Living Freeze engine already has the desired capture duration, safe read regions, overlapping voices, click-safe restarts, and a stable continuous texture at Drift=0. Smooth was layered onto this architecture through parameter plumbing, transient analysis/attenuation, a variable window, and gain-response changes. The current pitch implementation also allows each voice to undertake its own broad random walk, which makes the summed texture's phase relationship unstable.

The design must retain real-time safety: all voice state is prepared ahead of processing, and processing performs no allocation, locking, I/O, or blocking work.

## Goals / Non-Goals

**Goals:**

- Remove every Smooth-facing and Smooth-only DSP path while retaining the stable baseline texture engine.
- Keep Drift=0 as an exact pitch-identity baseline: global and local factors resolve to `1.0`.
- Make slow global pitch movement the dominant source of pitch drift, with constrained local variation for gentle beating.
- Preserve independent, bounded, smoothed position and stereo drift.
- Make pitch state deterministic under a supplied test seed and continuous across voice restarts.

**Non-Goals:**

- Changing the capture length, overlap count, safe-region strategy, or replacing the texture with a single-loop reader.
- Adding a user-controllable window shape, modulation mode, compressor, limiter, transient suppressor, or gain-riding stage.
- Guaranteeing backwards restoration of a removed Smooth parameter from legacy host state.

## Decisions

### 1. Remove Smooth at the parameter boundary and delete dependent state

The parameter layout, APVTS lookup/attachment, processor-to-engine call, editor slider/label/layout, state expectations, and tests will contain only Freeze, Drift, Dry/Wet, and Output Gain. The engine API will no longer accept Smooth.

Frozen transient marker storage/capture analysis, source transient scaling, wet-signal attack detection, transient envelopes/gains, and Smooth-controlled gain-compensation time constants will be deleted rather than bypassed. Captured audio remains an unmodified copy of the recent audio fragment.

This is preferred over retaining hidden Smooth defaults because a dormant path can still affect state compatibility, processing, and future tuning.

### 2. Restore one fixed baseline window

Every voice will use the existing stable non-Smooth envelope, `sin(pi * phase)` or its exact equivalent, without a control-dependent blend. It reaches zero at each boundary and retains smooth fade-in/out, so the established near-zero-gain restart discipline and overlap architecture remain valid.

This is preferred over inventing a replacement adaptive window because the change deliberately isolates pitch coherence from transient suppression and must preserve the proven Drift=0 behavior.

### 3. Model pitch as shared speed multiplied by a small local factor

The engine will own persistent texture-level state, conceptually `globalPlaybackSpeed` and `globalPlaybackSpeedTarget`. At each slow random-walk update interval, a seeded random source creates a bounded new global target based on the nonlinear pitch amount. The current global speed approaches that target under a dedicated slow pitch smoother (initial target: about 0.8 s; tune only within 0.8-1.5 s if listening/tests require it).

Each voice retains a persistent local pitch factor and optional local target. Local factors change only slowly and are smoothed; their permitted spread is intentionally much smaller than the global range. The final speed is composed as:

`voicePlaybackSpeed = globalPlaybackSpeed * voicePitchFactor`

The initial tuning target is approximately 85-95% shared movement and 5-15% local variation. At zero Drift, the global target/current speed and every local factor/target are explicitly set or converge to `1.0`, so no residual random pitch remains.

Multiplicative composition preserves a coherent common pitch trend across voices and gives a scale-independent way to bound subtle detune. It is preferred over independent per-voice full-range random walks, which cause correlated energy events in the summed signal.

### 4. Preserve pitch state through voice restart

Global pitch state belongs to the texture, not to any voice, and never changes as a consequence of a restart. A voice retains its current local factor when it gets a new safe start position. If the restart schedules a new local target, it keeps the current factor and glides gradually rather than resetting to unity or a new target.

This is preferred over generating a fresh per-restart pitch value because a new value would turn the restart cadence into audible pitch events despite window click safety.

### 5. Keep gain compensation fixed and energy-aware

The existing stable texture compensation remains, but its response uses one fixed conservative smoothing behavior. It is based only on active fixed-window contributions; it has no transient-marker input, source attenuation, wet attack detection, or Drift/Smooth-dependent rate. The implementation may tune its fixed constants only if the rendered Drift sweep reveals a material RMS loss.

This is preferred over transient-dependent attenuation or fast normalisation because those approaches disguise the artefact as level pumping and alter the intended texture dynamics.

### 6. Make tests inspect deterministic pitch state as well as audio output

Expose or retain narrowly scoped engine test observability for seeded global target/current speed, local pitch factors, and voice speeds. Unit tests will confirm zero-drift unity values; dominant common trend/local bounds; nonlinear low-drift scaling; gradual response to a target update; restart continuity; long-render finiteness, speed bounds, and discontinuity limits; and comparable long-term RMS across Drift levels.

This supplements audio-only assertions, which can be sensitive to source material and make coherence failures difficult to diagnose.

## Risks / Trade-offs

- [Exact global/local percentage needs listening adjustment] → Start with the documented dominant-global ratio, define bounded local ranges in tests, and tune only after comparable chord renders.
- [Pitch drift changes safe read excursion] → Continue to calculate safe regions using the final bounded speed range and restart only at near-zero envelope gain.
- [Legacy sessions retain an unknown Smooth state property] → Remove the parameter from the current layout; JUCE state restoration must safely ignore the absent parameter rather than treating it as required.
- [Global motion can sound too correlated/static] → Preserve small independent local factors and existing position/stereo drift, without widening local pitch range enough to recreate the problem.
- [RMS can vary naturally with decorrelation] → Compare long-term RMS with a conservative tolerance; reject only large systematic level loss and avoid automatic gain-riding fixes.

## Migration Plan

1. Remove Smooth parameter identifiers and UI/state references, allowing restored legacy state to omit the no-longer-defined parameter safely.
2. Delete Smooth-only transient and variable-window DSP state, retaining the capture and overlap data path.
3. Introduce/reset global and local pitch state in engine lifecycle methods, then replace independent voice pitch walks with the composed-speed model.
4. Add/adjust JUCE tests before final tuning; build and run the complete test suite and manual chord listening comparisons at Drift 0%, 20%, 50%, and 100%.
5. Rollback, if needed before release, by reverting this cohesive change; no persisted data migration is required because Smooth is intentionally a breaking removal.
