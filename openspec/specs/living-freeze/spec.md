# living-freeze Specification

## Purpose

This capability turns a recent stereo audio moment into an indefinitely sustaining texture whose movement remains smooth, subtle at low Drift, and non-periodic rather than sounding like a static loop.

## Requirements

### Requirement: Live passthrough and recent material
The processor SHALL pass the live stereo input through as the source signal when Freeze is disabled and SHALL continuously retain enough recent audio to capture a fixed, bounded fragment immediately preceding a Freeze activation. The capture duration SHALL be selected within approximately 500-750 ms and SHALL be independent of the total recent-history capacity.

#### Scenario: Unfrozen signal passes through
- **WHEN** Freeze is disabled and Dry/Wet is set to 0%
- **THEN** the output matches the current stereo input within normal floating-point processing tolerance

#### Scenario: Recent audio is available for capture
- **WHEN** Freeze is enabled after audio has been received
- **THEN** the processor captures a non-empty fragment from recently received input without requiring new input to replace it

#### Scenario: Recent audio is available for bounded capture
- **WHEN** Freeze is enabled after more than the configured capture duration of audio has been received
- **THEN** the processor captures a non-empty fragment whose length is approximately the configured duration and whose samples come from immediately before the Freeze trigger

#### Scenario: Capture does not include older history
- **WHEN** audio older than the configured capture window differs materially from the audio immediately preceding Freeze
- **THEN** the captured fragment excludes that older audio

#### Scenario: Capture is not extended by unavailable history
- **WHEN** Freeze is enabled before the configured capture duration of audio has been received
- **THEN** the processor captures only the available preceding audio and does not read uninitialized or future samples

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
When Freeze is enabled, the processor SHALL use at least two simultaneously active playback windows or equivalent read-head voices. Each voice SHALL use one fixed, non-negative, continuous baseline amplitude envelope equivalent to `sin(pi * phase)`, with gain zero or near zero at both window boundaries and smooth fade-in/out. A voice SHALL begin at a position within a safe continuously readable region of the captured fragment and SHALL overlap another voice while fading out so hard starts and stops are not exposed. The source material SHALL remain recognizable when Drift is zero or low.

#### Scenario: Multiple voices are active
- **WHEN** a valid fragment is available and Freeze remains enabled
- **THEN** at least two playback windows contribute over the sustained texture period, with independent read progress or window phase

#### Scenario: Window boundaries are soft
- **WHEN** an individual playback window begins or ends
- **THEN** its contribution changes continuously through the fixed non-negative baseline envelope, with gain near zero at both boundaries, and does not cause a large immediate sample jump

#### Scenario: Voice read range is safe
- **WHEN** a voice is scheduled for a playback window at any supported speed, Drift value, and stereo/position offset
- **THEN** its complete planned read excursion remains inside the captured fragment's safe continuous region and does not require cyclic buffer wrapping while its envelope is audible

#### Scenario: Low-drift material remains recognizable
- **WHEN** a held tone or chord is frozen with Drift at zero or a low positive value
- **THEN** the output remains recognizably derived from that tone or chord while reducing the clear short-loop character

### Requirement: Click-free transitions and boundaries
The processor SHALL smooth Freeze activation, Freeze deactivation, internal playback-boundary transitions, read-position changes, and Drift-induced movement so normal operation does not expose abrupt sample discontinuities. Capture-window start and end SHALL be joined using crossfading, overlapping windows, or an equivalent continuous-boundary method. A voice read position SHALL only be changed or restarted when that voice's audible envelope gain is at or near zero. Global and local playback-speed changes SHALL use dedicated slower pitch smoothing, independent of position and stereo smoothing, with a nominal design target of approximately 0.8 seconds and an allowed tuning range of 0.8-1.5 seconds. A voice restart SHALL preserve the current global pitch state and SHALL NOT reset its audible playback speed to `1.0`. Changes to Drift-dependent position limits SHALL also be applied continuously while a voice is audible.

#### Scenario: Freeze activation is smoothed
- **WHEN** Freeze changes from disabled to enabled during ordinary audio playback
- **THEN** the output transition contains no large immediate discontinuity attributable to a hard buffer switch or simultaneous hard voice start

#### Scenario: Freeze deactivation is smoothed
- **WHEN** Freeze changes from enabled to disabled
- **THEN** the output returns toward the live signal without a large immediate discontinuity and all texture voices fade out as part of the transition

#### Scenario: Internal boundaries are smoothed
- **WHEN** living-freeze playback reaches the end of a voice's safe read range
- **THEN** the voice has already faded to near-zero gain and adjacent output samples remain continuous within the processor's defined transition tolerance

#### Scenario: Voice restart is inaudible and pitch-continuous
- **WHEN** a voice completes its window and is assigned a new safe start position
- **THEN** the position change occurs only at near-zero envelope gain, the next cycle begins with a near-zero gain before fading in, and its playback speed does not make a large reset-related discontinuity

#### Scenario: Voice restart is inaudible
- **WHEN** a voice completes its window and is assigned a new safe start position
- **THEN** the position change occurs only at near-zero envelope gain and the next cycle begins with a near-zero gain before fading in

#### Scenario: Playback speed approaches a new target continuously
- **WHEN** the shared texture pitch target or a local voice pitch target changes
- **THEN** the affected playback speed does not jump directly to the target and successive processed samples change by small, continuous increments under the dedicated pitch smoothing response

#### Scenario: Drift changes do not cause an immediate pitch jump
- **WHEN** the visible Drift value changes substantially while Freeze remains enabled
- **THEN** the allowed playback-speed range and the current playback speed evolve continuously toward their new values without an abrupt pitch transition

#### Scenario: Drift changes do not cause an immediate read-position jump
- **WHEN** the visible Drift value changes substantially while a texture voice is audible
- **THEN** any changed position or stereo safety limit is approached continuously and does not instantly relocate the audible read position

### Requirement: Non-periodic living Drift
The processor SHALL apply smoothed, bounded, non-periodic, slowly varying Drift according to separate internal position, stereo, and pitch intensities derived from the normalized Drift value. Position and stereo movement SHALL be allowed to respond at low Drift values, while pitch movement SHALL use a nonlinear mapping whose lower range is substantially flatter than linear and whose middle range produces clearly more movement than the preceding response. Pitch drift SHALL consist primarily of one deterministic-for-a-fixed-seed shared global playback-speed random walk whose target updates slowly and whose current value approaches that target continuously; it SHALL not be a cyclic sinusoidal LFO or abrupt random jump. At maximum Drift, the shared global playback-speed target SHALL remain within +/-6% of unity, and each voice SHALL compose that shared speed with only a small, slowly changing local pitch factor no greater than +/-0.5% from unity. The global component SHALL dominate the total pitch movement. Position movement SHALL remain bounded to no more than 60 ms at maximum Drift, and Drift SHALL NOT move an active voice outside its safe continuous read region.

#### Scenario: Zero Drift preserves identity
- **WHEN** Drift is 0% and Freeze is enabled
- **THEN** multiple overlapping playback windows remain active with stable, predictable behavior, global playback speed and every voice playback speed remain effectively `1.0`, no intentional pseudo-random pitch, position, or stereo modulation is applied, and the output is not replaced by a classic single-loop read head

#### Scenario: Positive Drift evolves as one texture
- **WHEN** Drift is greater than 0% and Freeze remains enabled over time
- **THEN** the frozen output changes gradually through deterministic-for-a-fixed-seed, non-periodic position/stereo movement and a dominant shared pitch trend, while avoiding abrupt modulation jumps

#### Scenario: Positive Drift evolves the texture
- **WHEN** Drift is greater than 0% and Freeze remains enabled over time
- **THEN** the frozen output changes gradually through deterministic-for-a-fixed-seed, non-periodic movement while avoiding abrupt modulation jumps

#### Scenario: Local variation remains subordinate
- **WHEN** multiple texture voices are active at a positive Drift value
- **THEN** their playback speeds follow the same dominant global trend and their local pitch-factor differences remain small enough to provide gentle beating without independently detuned pitch excursions

#### Scenario: Low Drift remains subtle
- **WHEN** Drift is set to a low positive value such as 10-20%
- **THEN** the texture exhibits small measurable position and/or stereo movement while pitch remains effectively stable and does not produce a prominent pitch sweep or transient-like transition

#### Scenario: Mid Drift is clearly expressive
- **WHEN** Drift is set to 50% and Freeze remains enabled over time
- **THEN** the texture exhibits measurable smooth pitch and position movement that is stronger than the prior response at the same normalized control value, without abrupt pitch transitions

#### Scenario: Pitch mapping is sublinear at low Drift
- **WHEN** the normalized Drift value is `0.2`
- **THEN** the maximum pitch-drift amount is substantially less than `0.2` times the maximum pitch-drift amount at normalized Drift `1.0`

#### Scenario: Maximum Drift uses the full range
- **WHEN** Drift is set to 100% and Freeze remains enabled
- **THEN** the engine may reach the maximum configured global playback-speed deviation while remaining inside each current voice's safe read region, and its output is measurably and substantially different from the corresponding Drift 0% output for identical frozen material

#### Scenario: Pitch targets remain bounded and coherent
- **WHEN** Drift is set anywhere in its supported range and multiple texture voices are active
- **THEN** every voice's playback speed remains within the configured bounds, all speed changes remain finite and continuous, local factors remain a small variation around the shared component, and voice restarts do not introduce hard pitch jumps

#### Scenario: Pitch targets remain bounded and voice-independent
- **WHEN** Drift is set anywhere in its supported range and multiple texture voices are active
- **THEN** every voice's playback speed remains within the configured bounds, all speed changes remain finite and continuous, and voice-specific local targets do not introduce hard jumps

#### Scenario: Drift remains bounded
- **WHEN** Drift is set anywhere in its supported range
- **THEN** all affected playback properties remain within safe bounds, modulation changes remain continuous, and the output remains finite

### Requirement: Stable texture gain mixing
The processor SHALL mix active voice contributions using their actual non-negative fixed-window envelope gains and an energy-aware stable gain strategy that avoids unnecessary attenuation of decorrelated voices. Gain compensation SHALL use a fixed, slow response independent of Drift and SHALL not perform transient-dependent gain reduction, aggressive automatic normalization, or fast gain pumping. At 100% Dry/Wet and 0 dB Output Gain, the resulting freeze texture SHALL remain at a musically useful controlled level relative to the captured source and SHALL not become systematically negligible solely because of voice summation, windowing, gain compensation, or increasing Drift.

#### Scenario: Window gain remains audible
- **WHEN** one voice is active and its window moves from start through midpoint to end
- **THEN** the voice contribution follows the fixed envelope, including a near-zero start, intended maximum near the midpoint, and near-zero end

#### Scenario: Single voice is not normalized to constant level
- **WHEN** exactly one voice contributes with an envelope value below its maximum
- **THEN** the output contribution remains reduced by that envelope value rather than being fully restored by normalization

#### Scenario: Voice overlap changes smoothly
- **WHEN** the number or envelope gains of contributing voices changes during normal texture playback
- **THEN** the compensated texture level changes without a large abrupt output jump and without introducing a new audible modulation artifact

#### Scenario: Decorrelated voices retain controlled loudness
- **WHEN** multiple voices play the same captured material with small independent position, stereo, or pitch differences
- **THEN** the sustained texture RMS level remains close to the captured material's controlled reference level and does not become substantially quieter solely because the voices are decorrelated

#### Scenario: Wet texture avoids systematic level loss across Drift
- **WHEN** a defined sustained signal is captured and rendered at Drift 0%, 50%, and 100% with Freeze enabled, Dry/Wet at 100%, and Output Gain at 0 dB
- **THEN** each texture remains audibly and measurably present at a controlled non-negligible level and higher Drift does not cause a large systematic RMS reduction

#### Scenario: Wet texture avoids systematic level loss
- **WHEN** a defined sustained signal is captured, Freeze is enabled, Dry/Wet is 100%, and Output Gain is 0 dB
- **THEN** the texture remains audibly and measurably present at a controlled non-negligible level without requiring positive Output Gain compensation

#### Scenario: Gain compensation remains bounded
- **WHEN** voice envelopes move through repeated overlap and restart cycles at any supported Drift value
- **THEN** gain compensation remains finite and within defined safety limits without clipping, runaway amplification, or rapid corrective gain changes

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
The audio processing path SHALL avoid heap allocation, blocking operations, file access, and mutex locks during the audio callback, and SHALL produce no NaN or infinite output values for valid input and parameter values. All voice state required for texture playback SHALL be prepared before audio processing begins, including any state needed to calculate safe read regions and stable gain mixing.

#### Scenario: Callback remains real-time safe
- **WHEN** the processor handles valid stereo blocks during normal operation
- **THEN** processing uses pre-prepared state without allocation, blocking, file I/O, or mutex acquisition in the callback

#### Scenario: Output remains finite
- **WHEN** valid finite input is processed with any supported parameter values
- **THEN** every output sample is finite
