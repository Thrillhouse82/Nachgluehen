## Context

See proposal.md for motivation. The current engine already uses overlapping window voices, safe read/start regions, smoothed position and pitch targets, and a shared global pitch random walk with small local voice factors. Its current 3.5% maximum global pitch deviation, 35 ms maximum position movement, and pitch response near `drift ^ 2.5` leave medium and upper Drift less expressive than intended.

## Goals / Non-Goals

**Goals:**

- Tune the existing Drift ranges and response curves so medium and high settings create stronger, smooth texture evolution.
- Retain one dominant shared pitch trend, slow pitch smoothing, bounded safe movement, and the existing stable texture gain approach.
- Make numerical safety and perceived-level regressions observable through deterministic JUCE tests.

**Non-Goals:**

- Refactor or extract a new Drift subsystem.
- Change exposed parameter ranges, state serialization, voice overlap/windowing, capture behavior, or restart behavior.
- Use faster target updates, hard random jumps, transient suppression, or automatic gain reduction to increase apparent movement.

## Decisions

### Tune range before event rate

Increase maximum global pitch deviation to a listening-selected value in the 5-6% range, global random-walk step to 0.008-0.010, and maximum position drift to 50-60 ms. Keep target updates in the existing slow 1.5-3.0 second range and pitch smoothing initially at 1.5 seconds. This makes movement more audible through range rather than reintroducing transient-like events. Faster random updates were considered and rejected because they directly undermine the desired smooth global behavior.

### Bring pitch response into the middle of the control range

Reduce the pitch response exponent from approximately 2.5 to a listening-selected value in the 1.8-2.2 range, initially 2.0. Retain a nonlinear lower region so 0-20% remains subtle. A linear mapping was rejected because it would make low Drift too pitch-active.

### Keep local detuning subordinate

If needed, increase local pitch range only to 0.45-0.5% and its random-walk step only to 0.0005 or less. Local factors continue to compose with the global speed after their own slow smoothing. Increasing independent local movement was rejected because it changes the texture from one unstable layer into separately wandering voices.

### Make position and stereo carry more evolution safely

Use the larger bounded position range as the principal non-pitch contribution. Keep stereo movement tied to the existing movement amount; only reduce its exponent from 1.35 toward 1.15 if listening confirms that the increased position range is insufficient. Existing safety-limit recalculation and continuous target constraining remain responsible for ensuring that the enlarged offset cannot cause wrapping or an audible relocation.

### Verify behavior through deterministic diagnostic tests and renders

Extend JUCE tests using a fixed random seed and frozen source. Assert exact zero-Drift neutral factors, bounded global/local targets and safe positions, measurable stronger mid-Drift amounts, and non-immediate speed convergence. Render representative long textures at 0, 0.25, 0.5, 0.75, and 1.0 Drift to check finite samples, bounded values, discontinuity thresholds, and relative RMS stability. Listening determines the final value within each approved tuning range; tests protect the settled behavior thereafter.

## Risks / Trade-offs

- [A wider position offset leaves less safe playable region for short captures] → Retain conservative read/start reserves and add position-bound tests at every Drift level.
- [A stronger pitch curve can make low Drift too active] → Keep a nonlinear exponent no lower than the selected listening threshold and assert a subdued 20% response.
- [More decorrelation can lower wet RMS] → Preserve existing gain compensation and compare long-term RMS across all test settings without adding fast normalization.
- [Stronger random-walk steps can reveal speed changes] → Preserve slow target cadence and dedicated pitch smoothing, and test that targets are never applied as sample-level jumps.

## Migration Plan

No data or compatibility migration is required. The internal tuning values can be changed and tested in one implementation change; rollback consists of restoring the prior constants and response mapping if listening or regression tests fail.
