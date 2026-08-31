## 1. Remove Smooth contract and interface

- [x] 1.1 Remove the Smooth parameter definition, layout entry, processor reads, engine argument, state expectations, and Smooth-specific unit-test helpers/assertions.
- [x] 1.2 Remove the Smooth slider, label, attachment, and layout handling; verify the editor presents Freeze, Drift, Dry/Wet, and Output Gain cleanly at its fixed size.
- [x] 1.3 Ensure restoring legacy state that contains Smooth remains safe while Smooth is absent from the current parameter layout.

## 2. Simplify the texture DSP

- [x] 2.1 Delete frozen transient-marker capture/analysis and retain only the original captured audio fragment.
- [x] 2.2 Delete source transient attenuation, rendered-wet transient detection/envelope/gain attenuation, and Smooth-dependent gain-compensation behavior.
- [x] 2.3 Use the fixed baseline `sin(pi * phase)` window (or exact stable equivalent) for every voice and retain safe read/start regions, overlap scheduling, and near-zero-gain restarts.
- [x] 2.4 Retain or conservatively tune one fixed slow energy-aware texture gain-compensation response without compressor, limiter, transient suppression, or fast gain riding.

## 3. Implement coherent pitch drift

- [x] 3.1 Add prepared texture-level global playback-speed current/target state, seeded random-walk timing, bounded target generation, and dedicated slow pitch smoothing.
- [x] 3.2 Replace full independent voice pitch random walks with persistent, bounded, slowly smoothed local pitch factors composed with the global speed.
- [x] 3.3 Enforce Drift=0 unity global/local pitch state and retain existing independent bounded position and stereo movement for positive Drift.
- [x] 3.4 Preserve global and local pitch continuity through voice restarts; never reset audible pitch speed to unity or abruptly re-randomize a local factor.
- [x] 3.5 Confirm all updated modulation state remains allocation-free and lock-free in the audio callback and continues to respect speed-derived safe read bounds.

## 4. Add JUCE regression and behavior coverage

- [x] 4.1 Update parameter/editor/state tests to prove Smooth, Smooth attachments, Smooth engine inputs, frozen transient buffers, and transient gain-reduction state are absent.
- [x] 4.2 Add deterministic Drift=0 regression tests for unity global/voice speeds, unity targets/factors, finite stable texture output, and bounded sample discontinuities.
- [x] 4.3 Add seeded pitch-coherence and local-range tests showing common dominant global movement, small voice-factor spread, and substantially smaller pitch deviation at Drift 0.2 than at Drift 1.0.
- [x] 4.4 Add pitch-smoothing and repeated voice-restart tests proving targets are approached gradually and global/voice speed has no reset-related discontinuity.
- [x] 4.5 Add Drift 0/0.5/1.0 long-term RMS comparison plus long changing-Drift render coverage for finite output, bounded speeds, and no extreme discontinuities.

## 5. Verify and tune

- [x] 5.1 Build and run the complete JUCE unit-test suite, resolving regressions without reintroducing attenuation-based artefact suppression.
- [x] 5.2 Perform manual listening renders from the same captured chord at Drift 0%, 20%, 50%, and 100%; tune only bounded global/local pitch constants or fixed gain compensation as needed.
- [x] 5.3 Confirm the final behavior meets the regression baseline: Drift=0 remains calm and continuous, while high Drift is shared, tape-like, fluid, and free of major pumping or hard pitch events.
