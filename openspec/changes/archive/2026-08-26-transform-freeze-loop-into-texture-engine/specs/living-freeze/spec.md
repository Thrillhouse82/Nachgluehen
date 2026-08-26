## MODIFIED Requirements

### Requirement: Indefinite living freeze
When Freeze is enabled, the processor SHALL retain the captured fragment as the source of its output indefinitely, independent of subsequent input, and SHALL generate a continuous texture from multiple overlapping playback windows rather than treating the fragment as one complete repeating loop. The texture SHALL remain audible without artificial silent intervals, and no single fixed capture start shall recur as a regular attack pattern.

#### Scenario: Captured material survives input changes
- **WHEN** a fragment is captured and subsequent input changes to a materially different signal
- **THEN** the frozen output continues to be derived from the captured fragment rather than the new input

#### Scenario: Freeze playback continues
- **WHEN** Freeze remains enabled for longer than the captured fragment duration
- **THEN** the output remains audible and continuous through overlapping playback windows without stopping or inserting an artificial silent region at the original fragment boundary

#### Scenario: Continuous signal remains continuous across repetitions
- **WHEN** the captured fragment contains a continuously signal-bearing stereo source and Freeze remains enabled for many capture lengths
- **THEN** the output contains no periodic extended silent sections attributable to window transitions

#### Scenario: A marked capture impulse is not repeated as a classic loop
- **WHEN** the captured fragment contains a clearly marked single impulse near its beginning and Freeze remains enabled
- **THEN** the output does not reproduce that impulse as the same complete-buffer pattern at every captured-length interval

### Requirement: Overlapping window playback
When Freeze is enabled, the processor SHALL use at least two simultaneously active playback windows or equivalent read-head voices. Each voice SHALL use a soft continuous amplitude envelope, SHALL begin at a position within the captured fragment, and SHALL overlap with another voice while fading out so that hard voice starts and stops are not exposed. The source material SHALL remain recognizable when Drift is zero or low.

#### Scenario: Multiple voices are active
- **WHEN** a valid fragment is available and Freeze remains enabled
- **THEN** at least two playback windows contribute over the sustained texture period, with independent read progress or window phase

#### Scenario: Window boundaries are soft
- **WHEN** an individual playback window begins or ends
- **THEN** its contribution changes continuously through a soft envelope and does not cause a large immediate sample jump

#### Scenario: Low-drift material remains recognizable
- **WHEN** a held tone or chord is frozen with Drift at zero or a low positive value
- **THEN** the output remains recognizably derived from that tone or chord while reducing the clear short-loop character

### Requirement: Click-free transitions and boundaries
The processor SHALL smooth Freeze activation, Freeze deactivation, internal playback-boundary transitions, read-position changes, and Drift-induced movement so that normal operation does not expose abrupt sample discontinuities. Capture-window start and end SHALL be joined using crossfading, overlapping windows, or an equivalent continuous-boundary method.

#### Scenario: Freeze activation is smoothed
- **WHEN** Freeze changes from disabled to enabled during ordinary audio playback
- **THEN** the output transition contains no large immediate discontinuity attributable to a hard buffer switch or simultaneous hard voice start

#### Scenario: Freeze deactivation is smoothed
- **WHEN** Freeze changes from enabled to disabled
- **THEN** the output returns toward the live signal without a large immediate discontinuity and all texture voices fade out as part of the transition

#### Scenario: Internal boundaries are smoothed
- **WHEN** living-freeze playback crosses or changes an internal window read boundary
- **THEN** adjacent output samples remain continuous within the processor's defined transition tolerance and no click is introduced by the boundary

### Requirement: Non-periodic living Drift
The processor SHALL apply smoothed, bounded, non-periodic, slowly varying Drift according to the normalized Drift intensity. Drift MAY affect voice read positions, window starts or lengths, playback speeds, pitch-related speed, stereo placement, and voice timing, but changes SHALL remain continuous, finite, and free of hard modulation jumps. Drift SHALL not be implemented as only a simple cyclic sinusoidal LFO.

#### Scenario: Zero Drift preserves identity
- **WHEN** Drift is 0% and Freeze is enabled
- **THEN** multiple overlapping playback windows remain active with stable, predictable behavior, no intentional pseudo-random modulation is applied, and the output is not replaced by a classic single-loop read head

#### Scenario: Positive Drift evolves the texture
- **WHEN** Drift is greater than 0% and Freeze remains enabled over time
- **THEN** the frozen output changes gradually through deterministic-for-a-fixed-seed, non-periodic movement while avoiding abrupt modulation jumps

#### Scenario: Low Drift remains subtle
- **WHEN** Drift is set to a low positive value
- **THEN** the source material remains clearly recognizable while the output exhibits a measurable but small change over time

#### Scenario: Maximum Drift uses the full range
- **WHEN** Drift is set to 100% and Freeze remains enabled
- **THEN** the engine uses its maximum configured texture movement intensity and its output is measurably and substantially different from the corresponding Drift 0% output for identical frozen material

#### Scenario: Drift remains bounded
- **WHEN** Drift is set anywhere in its supported range
- **THEN** all affected playback properties remain within safe bounds, modulation changes remain continuous, and the output remains finite

### Requirement: Real-time-safe processing
The audio processing path SHALL avoid heap allocation, blocking operations, file access, and mutex locks during the audio callback, and SHALL produce no NaN or infinite output values for valid input and parameter values. All voice state required for texture playback SHALL be prepared before audio processing begins.

#### Scenario: Callback remains real-time safe
- **WHEN** the processor handles valid stereo blocks during normal operation
- **THEN** processing uses pre-prepared state without allocation, blocking, file I/O, or mutex acquisition in the callback

#### Scenario: Output remains finite
- **WHEN** valid finite input is processed with any supported parameter values
- **THEN** every output sample is finite
