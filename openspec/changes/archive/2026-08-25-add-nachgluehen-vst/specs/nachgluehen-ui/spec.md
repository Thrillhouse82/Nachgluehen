## Purpose

This capability provides a minimal, fixed-size custom interface that makes the Freeze, Drift, and Dry/Wet controls easy to understand while conveying the atmospheric Nachgluehen visual identity.

## ADDED Requirements

### Requirement: Visible controls
The editor SHALL present exactly the MVP controls needed for Freeze, Drift, and Dry/Wet: a dedicated toggle for Freeze and rotary controls for Drift and Dry/Wet.

#### Scenario: Controls are discoverable
- **WHEN** the plugin editor is opened
- **THEN** the user can identify and operate Freeze, Drift, and Dry/Wet without relying on a standard desktop checkbox for Freeze

#### Scenario: Controls reflect parameters
- **WHEN** a parameter changes through the UI or host automation
- **THEN** the corresponding control reflects the current value and state

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
