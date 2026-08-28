## MODIFIED Requirements

### Requirement: Click-free transitions and boundaries
The processor SHALL smooth Freeze activation, Freeze deactivation, internal playback-boundary transitions, read-position changes, and Drift-induced movement so that normal operation does not expose abrupt sample discontinuities. Capture-window start and end SHALL be joined using crossfading, overlapping windows, or an equivalent continuous-boundary method. A voice read position SHALL only be changed or restarted when that voice's audible envelope gain is at or near zero. Playback-Speed changes SHALL use a dedicated slower smoothing response than position and stereo movement, with a nominal design target in the range of 200-500 ms, so that speed targets are approached continuously rather than stepped. Changes to Drift-dependent position limits SHALL also be applied continuously while a voice is audible.

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

#### Scenario: Playback speed approaches a new target continuously
- **WHEN** a texture voice receives a different playback-speed target
- **THEN** playback speed does not jump directly to the target and successive processed samples change by small, continuous increments under the dedicated pitch smoothing response

#### Scenario: Drift changes do not cause an immediate pitch jump
- **WHEN** the visible Drift value changes substantially while Freeze remains enabled
- **THEN** the allowed playback-speed range and the current playback speed evolve continuously toward their new values without an abrupt pitch transition

#### Scenario: Drift changes do not cause an immediate read-position jump
- **WHEN** the visible Drift value changes substantially while a texture voice is audible
- **THEN** any changed position or stereo safety limit is approached continuously and does not instantly relocate the audible read position

### Requirement: Non-periodic living Drift
The processor SHALL apply smoothed, bounded, non-periodic, slowly varying Drift according to separate internal position, stereo, and pitch intensities derived from the normalized Drift value. Position and stereo movement SHALL be allowed to respond at low Drift values, while pitch-related playback-speed movement SHALL use a nonlinear mapping whose lower range is substantially flatter than linear. Pitch targets SHALL evolve through bounded small steps or an equivalent continuous random-walk behavior rather than repeatedly jumping across the full pitch range. Drift SHALL not be implemented as only a simple cyclic sinusoidal LFO and SHALL NOT move an active voice outside its safe continuous read region. Each voice MAY use an independent pitch target, but voice-to-voice pitch differences SHALL remain small at low and medium Drift values and MAY increase at high Drift values.

#### Scenario: Zero Drift preserves identity
- **WHEN** Drift is 0% and Freeze is enabled
- **THEN** multiple overlapping playback windows remain active with stable, predictable behavior, playback speed remains effectively `1.0`, no intentional pseudo-random pitch modulation is applied, and the output is not replaced by a classic single-loop read head

#### Scenario: Positive Drift evolves the texture
- **WHEN** Drift is greater than 0% and Freeze remains enabled over time
- **THEN** the frozen output changes gradually through deterministic-for-a-fixed-seed, non-periodic movement while avoiding abrupt modulation jumps

#### Scenario: Low Drift remains subtle
- **WHEN** Drift is set to a low positive value such as 10-20%
- **THEN** the texture exhibits small measurable position and/or stereo movement while pitch remains effectively stable and does not produce a prominent pitch sweep or transient-like transition

#### Scenario: Pitch mapping is sublinear at low Drift
- **WHEN** the normalized Drift value is `0.2`
- **THEN** the maximum pitch-drift amount is substantially less than `0.2` times the maximum pitch-drift amount at normalized Drift `1.0`

#### Scenario: Maximum Drift uses the full range
- **WHEN** Drift is set to 100% and Freeze remains enabled
- **THEN** the engine may reach the maximum configured playback-speed deviation while remaining inside the current voice's safe read region, and its output is measurably and substantially different from the corresponding Drift 0% output for identical frozen material

#### Scenario: Pitch targets remain bounded and voice-independent
- **WHEN** Drift is set anywhere in its supported range and multiple texture voices are active
- **THEN** every voice's playback speed remains within the configured bounds, all speed changes remain finite and continuous, and voice-specific pitch targets do not introduce hard jumps

#### Scenario: Drift remains bounded
- **WHEN** Drift is set anywhere in its supported range
- **THEN** all affected playback properties remain within safe bounds, modulation changes remain continuous, and the output remains finite

### Requirement: Stable texture gain mixing
The processor SHALL mix active voice contributions using their actual non-negative window envelope gains and an energy-aware stable gain strategy that avoids unnecessary attenuation of decorrelated voices. Gain compensation SHALL change slowly enough to avoid abrupt samplewise gain jumps or audible pumping at expected voice overlap and restart points. The resulting freeze texture SHALL remain within a controlled level range relative to the captured source and SHALL not rely on instantaneous envelope-sum division as its only loudness model.

#### Scenario: Window gain remains audible
- **WHEN** one voice is active and its window moves from start through midpoint to end
- **THEN** the voice contribution follows the envelope, including a near-zero start, intended maximum near the midpoint, and near-zero end

#### Scenario: Single voice is not normalized to constant level
- **WHEN** exactly one voice contributes with an envelope value below its maximum
- **THEN** the output contribution remains reduced by that envelope value rather than being fully restored by normalization

#### Scenario: Voice overlap changes smoothly
- **WHEN** the number or envelope gains of contributing voices changes during normal texture playback
- **THEN** the compensated texture level changes without a large abrupt output jump and without introducing a new audible modulation artifact

#### Scenario: Decorrelated voices retain controlled loudness
- **WHEN** multiple voices play the same captured material with small independent position, stereo, or pitch differences
- **THEN** the sustained texture RMS level remains close to the captured material's controlled reference level and does not become substantially quieter solely because the voices are decorrelated

#### Scenario: Gain compensation remains bounded
- **WHEN** voice envelopes move through repeated overlap and restart cycles at any supported Drift value
- **THEN** gain compensation remains finite and within defined safety limits without clipping or runaway amplification
