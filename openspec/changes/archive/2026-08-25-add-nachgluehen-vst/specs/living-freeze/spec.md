## Purpose

This capability turns a recent stereo audio moment into an indefinitely sustaining texture whose movement remains smooth, subtle at low Drift, and non-periodic rather than sounding like a static loop.

## ADDED Requirements

### Requirement: Live passthrough and recent material
The processor SHALL pass the live stereo input through as the source signal when Freeze is disabled and SHALL continuously retain enough recent audio to capture a useful fragment when Freeze is enabled.

#### Scenario: Unfrozen signal passes through
- **WHEN** Freeze is disabled and Dry/Wet is set to 0%
- **THEN** the output matches the current stereo input within normal floating-point processing tolerance

#### Scenario: Recent audio is available for capture
- **WHEN** Freeze is enabled after audio has been received
- **THEN** the processor captures a non-empty fragment from recently received input without requiring new input to replace it

### Requirement: Indefinite living freeze
When Freeze is enabled, the processor SHALL retain the captured fragment as the source of its output indefinitely, independent of subsequent input, and SHALL support continuous playback without a hard audible loop boundary.

#### Scenario: Captured material survives input changes
- **WHEN** a fragment is captured and subsequent input changes to a materially different signal
- **THEN** the frozen output continues to be derived from the captured fragment rather than the new input

#### Scenario: Freeze playback continues
- **WHEN** Freeze remains enabled for longer than the captured fragment duration
- **THEN** the output remains audible and continuous without stopping at the original fragment boundary

### Requirement: Click-free transitions and boundaries
The processor SHALL smooth Freeze activation, Freeze deactivation, internal playback-boundary transitions, read-position changes, and Drift-induced movement so that normal operation does not expose abrupt sample discontinuities.

#### Scenario: Freeze activation is smoothed
- **WHEN** Freeze changes from disabled to enabled during ordinary audio playback
- **THEN** the output transition contains no large immediate discontinuity attributable to a hard buffer switch

#### Scenario: Freeze deactivation is smoothed
- **WHEN** Freeze changes from enabled to disabled
- **THEN** the output returns toward the live signal without a large immediate discontinuity

#### Scenario: Internal boundaries are smoothed
- **WHEN** living-freeze playback crosses or changes an internal read boundary
- **THEN** adjacent output samples remain continuous within the processor's defined transition tolerance

### Requirement: Non-periodic living Drift
The processor SHALL apply smoothed, non-periodic, slowly varying Drift according to the normalized Drift intensity, and SHALL not implement Drift as only a simple cyclic sinusoidal LFO.

#### Scenario: Zero Drift preserves identity
- **WHEN** Drift is 0% and Freeze is enabled
- **THEN** the frozen playback remains stable and no intentional Drift modulation is applied

#### Scenario: Positive Drift evolves the texture
- **WHEN** Drift is greater than 0% and Freeze remains enabled over time
- **THEN** the frozen output changes gradually through non-periodic movement while avoiding abrupt modulation jumps

#### Scenario: Maximum Drift uses the full range
- **WHEN** Drift is set to 100% and Freeze remains enabled
- **THEN** the engine uses its maximum configured Drift intensity and its output is measurably different from the corresponding Drift 0% output for identical frozen material

#### Scenario: Drift remains bounded
- **WHEN** Drift is set anywhere in its supported range
- **THEN** all affected playback properties remain within safe bounds and the output remains finite

### Requirement: Dry/Wet mixing
The processor SHALL mix the current live input and living-freeze signal according to Dry/Wet, with 0% fully dry and 100% fully wet, and SHALL smooth mix changes sufficiently to prevent unintended level jumps.

#### Scenario: Dry output
- **WHEN** Dry/Wet is 0%
- **THEN** the output contains only the original live input

#### Scenario: Wet output
- **WHEN** Dry/Wet is 100% and Freeze is enabled
- **THEN** the output contains only the living-freeze signal

#### Scenario: Intermediate mix
- **WHEN** Dry/Wet is between 0% and 100%
- **THEN** the output is a corresponding blend of live input and living-freeze signal without an activation-related level jump

### Requirement: Real-time-safe processing
The audio processing path SHALL avoid heap allocation, blocking operations, file access, and mutex locks during the audio callback, and SHALL produce no NaN or infinite output values for valid input and parameter values.

#### Scenario: Callback remains real-time safe
- **WHEN** the processor handles valid stereo blocks during normal operation
- **THEN** processing uses pre-prepared state without allocation, blocking, file I/O, or mutex acquisition in the callback

#### Scenario: Output remains finite
- **WHEN** valid finite input is processed with any supported parameter values
- **THEN** every output sample is finite
