## 1. Drift response tuning

- [x] 1.1 Tune the shared global pitch deviation and random-walk step within the approved ranges, preserving slow target updates and the current dedicated pitch smoothing behavior.
- [x] 1.2 Adjust the pitch response curve so 50% Drift is materially more expressive while 0-20% remains subtle and 100% reaches the bounded maximum range.
- [x] 1.3 Increase bounded position movement within the approved range and only adjust the movement/stereo response curve if needed after listening, retaining continuous safe-target constraining.
- [x] 1.4 Keep local voice pitch range and random-walk movement subordinate to the shared global component, using only the approved small increase if needed.

## 2. Automated regression coverage

- [x] 2.1 Add a deterministic JUCE test that verifies Drift 0 keeps global and local speed factors at unity with no intentional position or stereo movement.
- [x] 2.2 Add tests for maximum global pitch, local pitch, and position target/read safety bounds at maximum Drift.
- [x] 2.3 Add a mid-Drift response test that proves pitch and movement amounts exceed the prior mapping without requiring an exact loudness result.
- [x] 2.4 Add a pitch-smoothing test confirming that a changed target is approached continuously rather than creating an immediate playback-speed jump.
- [x] 2.5 Add long-render tests at Drift 0, 0.25, 0.5, 0.75, and 1.0 that verify finite output, bounded pitch/read values, and no strong discontinuities.
- [x] 2.6 Add a long-term RMS comparison across Drift values that rejects a large systematic wet-level loss while retaining the existing gain-compensation strategy.

## 3. Verification and listening

- [x] 3.1 Build and run the JUCE unit-test target, fixing any tuning or regression failures.
- [x] 3.2 Perform documented manual listening checks with the same frozen sustained chord, guitar/pluck, and soft pad sources across the requested Drift settings; settle final values within the approved ranges.
