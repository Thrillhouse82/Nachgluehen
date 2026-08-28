## MODIFIED Requirements

### Requirement: Stable texture gain mixing
The processor SHALL mix active voice contributions using their actual non-negative window envelope gains and an energy-aware stable gain strategy that avoids unnecessary attenuation of decorrelated voices. Gain compensation SHALL change slowly enough to avoid abrupt samplewise gain jumps or audible pumping at expected voice overlap and restart points. At 100% Dry/Wet and 0 dB Output Gain, the resulting freeze texture SHALL remain at a musically useful controlled level relative to the captured source and SHALL not become systematically negligible solely because of voice summation, windowing, or gain compensation.

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

#### Scenario: Wet texture avoids systematic level loss
- **WHEN** a defined sustained signal is captured, Freeze is enabled, Dry/Wet is 100%, and Output Gain is 0 dB
- **THEN** the texture remains audibly and measurably present at a controlled non-negligible level without requiring positive Output Gain compensation

#### Scenario: Gain compensation remains bounded
- **WHEN** voice envelopes move through repeated overlap and restart cycles at any supported Drift value
- **THEN** gain compensation remains finite and within defined safety limits without clipping or runaway amplification
