# plugin-parameters-and-state Specification

## Purpose

This capability defines the stable host-facing parameter and project-state contract for automating Nachgluehen and restoring its visible settings predictably across project reloads.

## Requirements

### Requirement: Initial parameter contract
The plugin SHALL expose Freeze as a boolean parameter defaulting to disabled, Drift as a continuous UI percentage parameter in the range 0-100% defaulting to 20%, and Dry/Wet as a continuous UI percentage parameter in the range 0-100% defaulting to 50%. Drift values SHALL map linearly to the normalized processing range 0.0-1.0.

#### Scenario: Defaults are available
- **WHEN** a new plugin instance is created
- **THEN** Freeze is off, Drift is 20%, and Dry/Wet is 50%

#### Scenario: Values stay in range
- **WHEN** a host or UI supplies parameter values
- **THEN** Drift and Dry/Wet are constrained to 0-100% and Freeze resolves to a boolean state

#### Scenario: Drift mapping is normalized
- **WHEN** the UI or host supplies Drift values of 0%, 50%, and 100%
- **THEN** the DSP receives normalized values of 0.0, 0.5, and 1.0 respectively

### Requirement: Host automation
The plugin SHALL expose Freeze, Drift, and Dry/Wet as host-automatable parameters whose changes affect processing predictably.

#### Scenario: Host automates Freeze
- **WHEN** the host changes Freeze during playback
- **THEN** the processor applies the corresponding freeze transition with the click-free behavior defined by the Living Freeze capability

#### Scenario: Host automates continuous controls
- **WHEN** the host automates Drift or Dry/Wet
- **THEN** the processor applies the new value without invalid output or an abrupt uncontrolled parameter jump

#### Scenario: Drift automation reaches the DSP
- **WHEN** Drift is automated across its full 0-100% UI range
- **THEN** the corresponding normalized value reaches the Living-Freeze engine and changes its smoothed non-periodic modulation intensity

### Requirement: State persistence
The plugin SHALL serialize and restore all visible parameter values, including Freeze, Drift, and Dry/Wet, in the plugin state.

#### Scenario: Visible settings are restored
- **WHEN** a state containing non-default visible parameter values is saved and loaded into a plugin instance
- **THEN** all three visible parameters match the saved values

#### Scenario: Frozen audio is not required in state
- **WHEN** a project is reopened with Freeze restored as enabled
- **THEN** the parameter state is restored safely even if the previously captured audio fragment is unavailable
