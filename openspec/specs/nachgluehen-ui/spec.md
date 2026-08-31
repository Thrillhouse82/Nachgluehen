# nachgluehen-ui Specification

## Purpose

This capability provides a minimal, fixed-size custom interface that makes the Freeze, Drift, and Dry/Wet controls easy to understand while conveying the atmospheric Nachgluehen visual identity.

## Requirements

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

### Requirement: Output clipping status
The editor SHALL show a compact red `CLIP` indicator in an explicit visible bounds area associated with the Output Gain control. It SHALL indicate that the final post-Output-Gain signal reached a level within 0.1 dB of full scale, remain visible for at least one second after the most recent detected peak, and return inactive without requiring a user action when that hold interval elapses. The status bounds SHALL be independent of the Output Gain rotary-control bounds.

#### Scenario: Near-full-scale output activates CLIP
- **WHEN** the final plugin output reaches the clip-warning threshold
- **THEN** the red `CLIP` indicator becomes visible in its Output-area bounds

#### Scenario: CLIP remains visible after a short peak
- **WHEN** a peak activates the `CLIP` indicator and output subsequently falls below the threshold
- **THEN** the indicator remains visible for at least one second before turning off

#### Scenario: Safe output leaves CLIP inactive
- **WHEN** the final plugin output remains below the clip-warning threshold for longer than the hold interval
- **THEN** the `CLIP` indicator is inactive

### Requirement: Atmospheric custom styling
The editor SHALL use a cohesive custom visual treatment rather than an unmodified default JUCE appearance, with subdued dark tones and/or gradients, restrained glow or abstract texture, and sufficient contrast for readable controls. Visible plugin and title text SHALL use the ASCII spelling Nachgluehen.

#### Scenario: Custom visual identity is present
- **WHEN** the editor is rendered
- **THEN** its background and controls visibly use the Nachgluehen atmospheric styling

#### Scenario: Readability is preserved
- **WHEN** atmospheric background elements are rendered behind the controls
- **THEN** labels, values, and control states remain legible and distinguishable

### Requirement: Fixed editor presentation
The editor SHALL use a fixed MVP size and SHALL present the complete Nachgluehen control layout at that size. Responsive resizing and resize-specific layout behavior are outside the MVP.

#### Scenario: Fixed editor opens
- **WHEN** the host opens the plugin editor
- **THEN** the editor uses its configured fixed dimensions and all knobs, labels, and the Freeze toggle are fully visible and usable
