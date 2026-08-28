## MODIFIED Requirements

### Requirement: Visible controls
The editor SHALL present the MVP controls needed for Freeze, Drift, Dry/Wet, and Output Gain: a dedicated toggle for Freeze and rotary controls for Drift, Dry/Wet, and Output Gain.

#### Scenario: Controls are discoverable
- **WHEN** the plugin editor is opened
- **THEN** the user can identify and operate Freeze, Drift, Dry/Wet, and Output Gain without relying on a standard desktop checkbox for Freeze

#### Scenario: Controls reflect parameters
- **WHEN** a parameter changes through the UI or host automation
- **THEN** the corresponding control reflects the current value and state

## ADDED Requirements

### Requirement: Output clipping status
The editor SHALL show a compact red `CLIP` indicator beside the output control. It SHALL indicate that the final post-Output-Gain signal reached a level within 0.1 dB of full scale, remain visible for at least one second after the most recent detected peak, and return inactive without requiring a user action when that hold interval elapses.

#### Scenario: Near-full-scale output activates CLIP
- **WHEN** the final plugin output reaches the clip-warning threshold
- **THEN** the red `CLIP` indicator becomes visible

#### Scenario: CLIP remains visible after a short peak
- **WHEN** a peak activates the `CLIP` indicator and output subsequently falls below the threshold
- **THEN** the indicator remains visible for at least one second before turning off

#### Scenario: Safe output leaves CLIP inactive
- **WHEN** the final plugin output remains below the clip-warning threshold for longer than the hold interval
- **THEN** the `CLIP` indicator is inactive
