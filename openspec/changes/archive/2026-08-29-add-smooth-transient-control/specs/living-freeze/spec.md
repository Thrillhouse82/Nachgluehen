## ADDED Requirements

### Requirement: Musical transient-character control
The processor SHALL use the normalized Smooth control to continuously vary the wet Living-Freeze texture between retaining transient-rich captured attack structures at 0% and substantially reducing repeated short attack-energy changes at 100%. Intermediate values SHALL provide musically useful intermediate behavior. Smooth SHALL affect only the frozen wet texture and SHALL NOT replace or disable Drift-dependent position, pitch, or stereo movement.

#### Scenario: Zero Smooth retains transient character
- **WHEN** a transient-rich fragment is frozen with Smooth at 0%
- **THEN** the resulting wet texture retains the engine's baseline rhythmic, granular, and attack-character behavior without unexpected additional transient smoothing

#### Scenario: Maximum Smooth reduces transient energy changes
- **WHEN** identical transient-rich material is frozen and rendered with Smooth at 0% and 100%
- **THEN** the 100% Smooth render has measurably smaller short-term peak or attack-energy variation than the 0% Smooth render

#### Scenario: Smooth retains Drift movement
- **WHEN** Smooth and Drift are both 100% during a sustained freeze
- **THEN** the texture continues to show bounded position, pitch, and/or stereo evolution while presenting reduced transient attacks

#### Scenario: Smooth affects only wet texture
- **WHEN** Freeze is disabled or Dry/Wet is 0%
- **THEN** changing Smooth does not alter the live dry signal

### Requirement: Smooth continuous voice boundaries and gain response
For every Smooth value, each texture voice SHALL retain a non-negative continuous envelope that reaches zero or near zero at both boundaries, and a voice SHALL only adopt a new safe buffer position at near-zero envelope gain. Higher Smooth values SHALL make newly restarted content less abruptly exposed and SHALL slow the response of texture gain compensation relative to lower Smooth values, without aggressive automatic normalization or rapid gain pumping.

#### Scenario: Window boundaries stay safe across Smooth values
- **WHEN** a voice window is evaluated at its start and end for any supported Smooth value
- **THEN** its envelope is zero or near zero at both boundaries and changes continuously between adjacent phase values

#### Scenario: Smooth restarts remain inaudible
- **WHEN** a high-Smooth voice completes a window and is assigned a new safe start position
- **THEN** the position change occurs at near-zero envelope gain and the next content fades in without a large sample discontinuity

#### Scenario: High Smooth gain response is stable
- **WHEN** high Smooth is used through repeated overlap and restart cycles
- **THEN** texture gain compensation remains finite and bounded and does not make strong short-term corrective changes at normal restart points

#### Scenario: Click safety is retained across controls
- **WHEN** long freeze renders use supported combinations of Smooth and Drift with transient-rich captures
- **THEN** the output remains finite and contains no new strong sample discontinuities attributable to window boundaries, restarts, or Smooth changes
