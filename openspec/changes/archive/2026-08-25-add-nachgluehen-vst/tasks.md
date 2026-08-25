## 1. Project Foundation

- [x] 1.1 Add the top-level CMake configuration, JUCE dependency integration, plugin metadata, and VST3 target for Nachgluehen.
- [x] 1.2 Add source/include organization for the plugin processor, editor, parameter/state layer, Living-Freeze engine, and shared testable DSP types.
- [x] 1.3 Add a separate JUCE Unit Test Framework executable and register it with CTest or the project test workflow.

## 2. Parameters and Plugin Shell

- [x] 2.1 Implement the Freeze, Drift, and Dry/Wet parameter tree with stable IDs, boolean/continuous types, ranges, and defaults of off/20%/50%.
- [x] 2.2 Implement plugin lifecycle and stereo bus configuration, including preparation for sample-rate and block-size changes.
- [x] 2.3 Implement parameter-tree serialization and restoration, excluding transient frozen audio and safely handling restored Freeze without a captured buffer.
- [x] 2.4 Connect host parameter changes to the DSP engine through bounded, sample-safe smoothing state.
- [x] 2.5 Correct the Drift parameter presentation and mapping so the UI covers 0-100% and normalized DSP values are 0.0-1.0 without double scaling.

## 3. Living-Freeze DSP Engine

- [x] 3.1 Implement preallocated recent-input stereo ring buffering and bounded fragment capture on Freeze activation.
- [x] 3.2 Implement preallocated frozen-fragment storage and indefinite stereo playback independent of subsequent live input.
- [x] 3.3 Implement overlapping window/grain playback with interpolation, bounded read positions, and crossfades at internal boundaries.
- [x] 3.4 Implement Freeze activation/deactivation crossfades and smooth Dry/Wet mixing without audio-callback allocation or locking.
- [x] 3.5 Implement seedable smoothed pseudo-random/random-walk drift with bounded position, grain/window, pitch, and stereo variation; ensure Drift 0% disables intentional movement.
- [x] 3.6 Add finite-value guards and safe fallback behavior for empty captures, invalid buffer conditions, and restored Freeze without transient audio.
- [x] 3.7 Verify and correct the parameter-to-DSP Drift wiring so Drift > 0% changes the frozen output measurably while remaining smoothed and non-periodic.

## 4. Custom Editor

- [x] 4.1 Implement a dedicated LookAndFeel and editor layout with one Freeze toggle and rotary Drift and Dry/Wet controls.
- [x] 4.2 Draw the atmospheric Nachgluehen background, subdued gradients/glow/texture, readable labels, values, and active/inactive states.
- [x] 4.3 Attach editor controls to the parameter tree.
- [x] 4.4 Replace responsive resize behavior with a fixed editor size and ensure all knobs, labels, values, and the Freeze toggle are fully visible at that size.

## 5. Automated Verification

- [x] 5.1 Test parameter defaults, ranges, host-facing IDs/types, and state serialization/restoration with JUCE Unit Tests.
- [x] 5.2 Test live passthrough, recent-material capture, frozen playback after input changes, and continued output beyond the source duration.
- [x] 5.3 Test Dry/Wet endpoints and intermediate mixing, including smooth Freeze transitions.
- [x] 5.4 Test stable Drift 0% playback, positive-drift evolution, bounded behavior, and deterministic output for identical test seeds.
- [x] 5.5 Test activation/deactivation and internal playback-boundary continuity against defined discontinuity tolerances.
- [x] 5.6 Test that representative stereo signals and parameter sweeps produce no NaN or infinite samples and that the test target runs through the build system.
- [x] 5.7 Add a Drift mapping test proving that UI/parameter values 0%, 50%, and 100% reach the DSP as 0.0, 0.5, and 1.0.
- [x] 5.8 Add endpoint and DSP response tests proving that Drift 0% is stable, Drift 100% is usable as the maximum range, and identical frozen input produces a measurable output difference when Drift > 0%.

## 6. Build and Manual Validation

- [x] 6.1 Build the VST3 and test targets on the supported development toolchain and run the complete automated test suite.
- [x] 6.2 Inspect the generated VST3 in Ableton Live or another compatible DAW, verify plugin loading, stereo processing, automation, state restoration, and manual click-free behavior.
- [x] 6.3 Manually evaluate the fixed-size atmospheric UI and subjective Drift/sound behavior in Ableton Live. Responsive resize and display-scaling behavior are outside the MVP.
