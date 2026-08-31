## MODIFIED Requirements

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
