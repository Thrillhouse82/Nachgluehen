## MODIFIED Requirements

### Requirement: Visible controls
The editor SHALL present the controls needed for Freeze, Drift, Dry/Wet, and Output Gain: a dedicated toggle for Freeze and equal-sized rotary controls for Drift, Dry/Wet, and Output Gain. Each rotary control SHALL use the same visual size and look-and-feel treatment; labels and status information SHALL NOT reduce any rotary control's size.

#### Scenario: Controls are discoverable
- **WHEN** the plugin editor is opened
- **THEN** the user can identify and operate Freeze, Drift, Dry/Wet, and Output Gain without relying on a standard desktop checkbox for Freeze

#### Scenario: Controls reflect parameters
- **WHEN** a parameter changes through the UI or host automation
- **THEN** the corresponding control reflects the current value and state

#### Scenario: Rotary controls have equal visual size
- **WHEN** the fixed editor layout is rendered
- **THEN** the Drift, Dry/Wet, and Output Gain controls have equal width, height, circular display size, and rotary look-and-feel presentation
