## MODIFIED Requirements

### Requirement: Initial parameter contract
The plugin SHALL expose Freeze as a boolean parameter defaulting to disabled, Drift as a continuous UI percentage parameter in the range 0-100% defaulting to 20%, Dry/Wet as a continuous UI percentage parameter in the range 0-100% defaulting to 50%, and Output Gain as a continuous dB parameter with a mute endpoint, a maximum of +12 dB, and a default of 0 dB. Drift values SHALL map linearly to the normalized processing range 0.0-1.0.

#### Scenario: Defaults are available
- **WHEN** a new plugin instance is created
- **THEN** Freeze is off, Drift is 20%, Dry/Wet is 50%, and Output Gain is 0 dB

#### Scenario: Values stay in range
- **WHEN** a host or UI supplies parameter values
- **THEN** Drift and Dry/Wet are constrained to 0-100%, Output Gain is constrained between its mute endpoint and +12 dB, and Freeze resolves to a boolean state

#### Scenario: Drift mapping is normalized
- **WHEN** the UI or host supplies Drift values of 0%, 50%, and 100%
- **THEN** the DSP receives normalized values of 0.0, 0.5, and 1.0 respectively

#### Scenario: Output Gain is applied after the mix
- **WHEN** Output Gain is set to 0 dB
- **THEN** the final output level is unchanged by the Output Gain stage after Dry/Wet mixing

#### Scenario: Output Gain can mute and boost
- **WHEN** Output Gain is set to its mute endpoint or to a positive value
- **THEN** the final output is respectively muted or amplified by the corresponding bounded gain without changing the Freeze, Drift, or Dry/Wet settings

### Requirement: Host automation
The plugin SHALL expose Freeze, Drift, Dry/Wet, and Output Gain as host-automatable parameters whose changes affect processing predictably.

#### Scenario: Host automates Freeze
- **WHEN** the host changes Freeze during playback
- **THEN** the processor applies the corresponding freeze transition with the click-free behavior defined by the Living Freeze capability

#### Scenario: Host automates continuous controls
- **WHEN** the host automates Drift, Dry/Wet, or Output Gain
- **THEN** the processor applies the new value without invalid output or an abrupt uncontrolled parameter jump

#### Scenario: Drift automation reaches the DSP
- **WHEN** Drift is automated across its full 0-100% UI range
- **THEN** the corresponding normalized value reaches the Living-Freeze engine and changes its smoothed non-periodic modulation intensity

#### Scenario: Output Gain automation is smoothed
- **WHEN** the host automates Output Gain during playback
- **THEN** the final output level approaches the new gain continuously without a zipper-noise-like discontinuity

### Requirement: State persistence
The plugin SHALL serialize and restore all visible parameter values, including Freeze, Drift, Dry/Wet, and Output Gain, in the plugin state.

#### Scenario: Visible settings are restored
- **WHEN** a state containing non-default visible parameter values is saved and loaded into a plugin instance
- **THEN** Freeze, Drift, Dry/Wet, and Output Gain match the saved values

#### Scenario: Frozen audio is not required in state
- **WHEN** a project is reopened with Freeze restored as enabled
- **THEN** the parameter state is restored safely even if the previously captured audio fragment is unavailable
