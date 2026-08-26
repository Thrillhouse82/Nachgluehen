## MODIFIED Requirements

### Requirement: Live passthrough and recent material
The processor SHALL pass the live stereo input through as the source signal when Freeze is disabled and SHALL continuously retain enough recent audio to capture a fixed, bounded fragment immediately preceding a Freeze activation. The capture duration SHALL be selected within approximately 500–750 ms and SHALL be independent of the total recent-history capacity.

#### Scenario: Unfrozen signal passes through
- **WHEN** Freeze is disabled and Dry/Wet is set to 0%
- **THEN** the output matches the current stereo input within normal floating-point processing tolerance

#### Scenario: Recent audio is available for bounded capture
- **WHEN** Freeze is enabled after more than the configured capture duration of audio has been received
- **THEN** the processor captures a non-empty fragment whose length is approximately the configured duration and whose samples come from immediately before the Freeze trigger

#### Scenario: Recent audio is available for capture
- **WHEN** Freeze is enabled after audio has been received
- **THEN** the processor captures a non-empty fragment from recently received input without requiring new input to replace it

#### Scenario: Capture does not include older history
- **WHEN** audio older than the configured capture window differs materially from the audio immediately preceding Freeze
- **THEN** the captured fragment excludes that older audio

#### Scenario: Capture is not extended by unavailable history
- **WHEN** Freeze is enabled before the configured capture duration of audio has been received
- **THEN** the processor captures only the available preceding audio and does not read uninitialized or future samples

### Requirement: Indefinite living freeze
When Freeze is enabled, the processor SHALL retain the captured fragment as the source of its output indefinitely, independent of subsequent input, and SHALL support continuous playback without a hard audible loop boundary or artificial silent interval between repetitions.

#### Scenario: Captured material survives input changes
- **WHEN** a fragment is captured and subsequent input changes to a materially different signal
- **THEN** the frozen output continues to be derived from the captured fragment rather than the new input

#### Scenario: Freeze playback continues
- **WHEN** Freeze remains enabled for longer than the captured fragment duration
- **THEN** the output remains audible and continuous without stopping or inserting an artificial silent region at the original fragment boundary

#### Scenario: Continuous signal remains continuous across repetitions
- **WHEN** the captured fragment contains a continuously signal-bearing stereo source and Freeze remains enabled for many repetitions
- **THEN** the output contains no periodic extended silent sections attributable to the loop implementation

### Requirement: Click-free transitions and boundaries
The processor SHALL smooth Freeze activation, Freeze deactivation, internal playback-boundary transitions, read-position changes, and Drift-induced movement so that normal operation does not expose abrupt sample discontinuities. Capture-window start and end SHALL be joined using crossfading, overlapping windows, or an equivalent continuous-boundary method.

#### Scenario: Freeze activation is smoothed
- **WHEN** Freeze changes from disabled to enabled during ordinary audio playback
- **THEN** the output transition contains no large immediate discontinuity attributable to a hard buffer switch

#### Scenario: Freeze deactivation is smoothed
- **WHEN** Freeze changes from enabled to disabled
- **THEN** the output returns toward the live signal without a large immediate discontinuity

#### Scenario: Internal boundaries are smoothed
- **WHEN** living-freeze playback crosses or changes an internal read boundary
- **THEN** adjacent output samples remain continuous within the processor's defined transition tolerance and no click is introduced by the boundary

### Requirement: Non-periodic living Drift
The processor SHALL apply smoothed, bounded, non-periodic, slowly varying Drift according to the normalized Drift intensity, using read-position movement and playback-speed movement large enough to be measurable and musically audible at high values. Drift SHALL not be implemented as only a simple cyclic sinusoidal LFO.

#### Scenario: Zero Drift preserves identity
- **WHEN** Drift is 0% and Freeze is enabled
- **THEN** the frozen playback remains stable and no intentional Drift modulation, read-position movement, or pitch movement is applied

#### Scenario: Positive Drift evolves the texture
- **WHEN** Drift is greater than 0% and Freeze remains enabled over time
- **THEN** the frozen output changes gradually through deterministic-for-a-fixed-seed, non-periodic movement while avoiding abrupt modulation jumps

#### Scenario: Low Drift remains subtle
- **WHEN** Drift is set to a low positive value
- **THEN** the source material remains clearly recognizable while the output exhibits a measurable but small change over time

#### Scenario: Maximum Drift uses the full range
- **WHEN** Drift is set to 100% and Freeze remains enabled
- **THEN** the engine uses its maximum configured read-position and playback-speed Drift intensity and its output is measurably and substantially different from the corresponding Drift 0% output for identical frozen material

#### Scenario: Drift remains bounded
- **WHEN** Drift is set anywhere in its supported range
- **THEN** all affected playback properties remain within safe bounds, modulation changes remain continuous, and the output remains finite
