## MODIFIED Requirements

### Requirement: Overlapping window playback
When Freeze is enabled, the processor SHALL use at least two simultaneously active playback windows or equivalent read-head voices. Each voice SHALL use a soft continuous amplitude envelope whose value is zero or near zero at both window boundaries, SHALL begin at a position within a safe continuously readable region of the captured fragment, and SHALL overlap with another voice while fading out so that hard voice starts and stops are not exposed. The source material SHALL remain recognizable when Drift is zero or low.

#### Scenario: Multiple voices are active
- **WHEN** a valid fragment is available and Freeze remains enabled
- **THEN** at least two playback windows contribute over the sustained texture period, with independent read progress or window phase

#### Scenario: Window boundaries are soft
- **WHEN** an individual playback window begins or ends
- **THEN** its contribution changes continuously through a non-negative soft envelope, with gain near zero at both boundaries, and does not cause a large immediate sample jump

#### Scenario: Voice read range is safe
- **WHEN** a voice is scheduled for a playback window at any supported speed, Drift value, and stereo/position offset
- **THEN** its complete planned read excursion remains inside the captured fragment's safe continuous region and does not require cyclic buffer wrapping while its envelope is audible

#### Scenario: Low-drift material remains recognizable
- **WHEN** a held tone or chord is frozen with Drift at zero or a low positive value
- **THEN** the output remains recognizably derived from that tone or chord while reducing the clear short-loop character

### Requirement: Click-free transitions and boundaries
The processor SHALL smooth Freeze activation, Freeze deactivation, internal playback-boundary transitions, read-position changes, and Drift-induced movement so that normal operation does not expose abrupt sample discontinuities. Capture-window start and end SHALL be joined using crossfading, overlapping windows, or an equivalent continuous-boundary method. A voice read position SHALL only be changed or restarted when that voice's audible envelope gain is at or near zero.

#### Scenario: Freeze activation is smoothed
- **WHEN** Freeze changes from disabled to enabled during ordinary audio playback
- **THEN** the output transition contains no large immediate discontinuity attributable to a hard buffer switch or simultaneous hard voice start

#### Scenario: Freeze deactivation is smoothed
- **WHEN** Freeze changes from enabled to disabled
- **THEN** the output returns toward the live signal without a large immediate discontinuity and all texture voices fade out as part of the transition

#### Scenario: Internal boundaries are smoothed
- **WHEN** living-freeze playback reaches the end of a voice's safe read range
- **THEN** the voice has already faded to near-zero gain and adjacent output samples remain continuous within the processor's defined transition tolerance

#### Scenario: Voice restart is inaudible
- **WHEN** a voice completes its window and is assigned a new safe start position
- **THEN** the position change occurs only at near-zero envelope gain and the next cycle begins with a near-zero gain before fading in

### Requirement: Non-periodic living Drift
The processor SHALL apply smoothed, bounded, non-periodic, slowly varying Drift according to the normalized Drift intensity. Drift MAY affect voice read positions, window starts or lengths, playback speeds, pitch-related speed, stereo placement, and voice timing, but changes SHALL remain continuous, finite, and free of hard modulation jumps. Drift SHALL not be implemented as only a simple cyclic sinusoidal LFO and SHALL NOT move an active voice outside its safe continuous read region.

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
- **THEN** the engine uses the maximum configured movement intensity that remains inside the current voice's safe read region and its output is measurably and substantially different from the corresponding Drift 0% output for identical frozen material

#### Scenario: Drift remains bounded
- **WHEN** Drift is set anywhere in its supported range
- **THEN** all affected playback properties and offsets remain within safe bounds, modulation changes remain continuous, and the output remains finite

### Requirement: Stable texture gain mixing
The processor SHALL mix active voice contributions using their actual non-negative window envelope gains and a stable gain strategy that avoids abrupt samplewise compensation. Gain compensation SHALL reduce level variation caused by expected voice overlap without dividing by the instantaneous sum of active envelopes in a way that restores a faded single voice to constant amplitude.

#### Scenario: Window gain remains audible
- **WHEN** one voice is active and its window moves from start through midpoint to end
- **THEN** the voice contribution follows the envelope, including a near-zero start, intended maximum near the midpoint, and near-zero end

#### Scenario: Single voice is not normalized to constant level
- **WHEN** exactly one voice contributes with an envelope value below its maximum
- **THEN** the output contribution remains reduced by that envelope value rather than being fully restored by normalization

#### Scenario: Voice overlap changes smoothly
- **WHEN** the number or envelope gains of contributing voices changes during normal texture playback
- **THEN** the compensated texture level changes without a large abrupt output jump and without introducing a new audible modulation artifact

### Requirement: Real-time-safe processing
The audio processing path SHALL avoid heap allocation, blocking operations, file access, and mutex locks during the audio callback, and SHALL produce no NaN or infinite output values for valid input and parameter values. All voice state required for texture playback SHALL be prepared before audio processing begins, including any state needed to calculate safe read regions and stable gain mixing.

#### Scenario: Callback remains real-time safe
- **WHEN** the processor handles valid stereo blocks during normal operation
- **THEN** processing uses pre-prepared state without allocation, blocking, file I/O, or mutex acquisition in the callback

#### Scenario: Output remains finite
- **WHEN** valid finite input is processed with any supported parameter values
- **THEN** every output sample is finite
