## 1. Capture Window

- [x] 1.1 Add a named fixed capture-duration constant within the specified 500–750 ms range and derive its sample count from the prepared sample rate.
- [x] 1.2 Update Freeze-edge capture to copy only the newest available capture-window samples from the recent ring buffer, preserving the immediately-preceding ordering and stereo channels.
- [x] 1.3 Ensure captured buffers and `capturedLength` remain immutable while Freeze is active, including when subsequent input changes.

## 2. Continuous Freeze Playback

- [x] 2.1 Adapt the loop-boundary crossfade/overlap to the bounded capture length, including safe behavior for short startup captures.
- [x] 2.2 Verify playback remains dense across repeated boundaries without artificial silent gaps, while retaining smoothed Freeze activation and deactivation.
- [x] 2.3 Keep all new runtime state preallocated and preserve finite-output protection in the audio callback.

## 3. Audible Non-Periodic Drift

- [x] 3.1 Replace the sample-scale read-position offset with a sample-rate-derived millisecond-scale maximum offset, bounded and smoothly ramped.
- [x] 3.2 Increase the bounded playback-speed/pitch Drift range and apply it through continuously smoothed cursor advancement.
- [x] 3.3 Preserve irregular seeded target generation, ensure Drift 0% bypasses intentional position and speed modulation, and verify deterministic behavior for a fixed seed.
- [x] 3.4 Inspect the single-read-head result against the specification and only introduce additional preallocated read-head state if the simpler design cannot produce the required measurable Drift response.

## 4. JUCE Unit Tests

- [x] 4.1 Add capture-length tests at a known sample rate that prove capture is duration-bounded, immediately precedes the Freeze trigger, and does not include older history.
- [x] 4.2 Add tests proving frozen material survives changed input and repeated playback produces no extended periodic silence for signal-bearing capture data.
- [x] 4.3 Add boundary and transition tests for finite output, loop discontinuity limits, and smoothed Freeze activation/deactivation.
- [x] 4.4 Add seeded Drift tests covering stable 0%, measurable low positive Drift, stronger high Drift, the 100% endpoint, deterministic reproduction, and bounded sample-to-sample changes.

## 5. Verification

- [x] 5.1 Build and run the existing JUCE Unit Test executable with no external test framework.
- [x] 5.2 Verify unchanged parameter identifiers, ranges, automation behavior, and state persistence through the existing processor tests.
- [x] 5.3 Perform manual Ableton Live listening at Drift 0%, low, medium, and 100% with representative sustained input, and adjust internal constants only while preserving the specification contract.
