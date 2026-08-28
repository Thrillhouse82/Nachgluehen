## Why

Die letzte Pitch-Drift-Anpassung reduziert harte Übergänge, die Freeze-Texture ist nun jedoch deutlich zu leise und bei größeren Drift-Werten weiterhin nicht vollständig frei von transientenartigen Pitch-/Read-Artefakten. Die bestehende flächige Voice-Struktur und Click-Safety sollen erhalten bleiben, während Lautheit und Bewegungscharakter musikalisch stabiler werden.

## What Changes

- Die Texture-Gain-Kompensation wird so angepasst, dass unkorrelierte oder leicht gegeneinander verschobene Voices nicht unnötig an wahrgenommener Lautheit verlieren.
- Die Pegelregelung bleibt langsam und stabil, ohne abrupte Gain-Sprünge oder Pumpen an Window-Grenzen.
- Pitch-Zielwechsel werden von großen zufälligen Sprüngen zu kleinen, begrenzten und langsam wandernden Änderungen weiterentwickelt.
- Pitch-Modulation reagiert bei niedriger und mittlerer Drift subtiler; hohe Drift darf weiterhin deutlich instabil klingen.
- Drift-bedingte Positions- und Safe-Read-Begrenzungen dürfen keine abrupten Read-Position-Sprünge erzeugen.
- Bestehende Window-Envelopes, Voice-Überlappung, Click-Safety, Echtzeitvorgaben und sichtbare Parameter bleiben erhalten.

## Capabilities

### New Capabilities

Keine.

### Modified Capabilities

- `living-freeze`: Anforderungen an stabile Texture-Lautheit, energiebezogene Voice-Mischung und besonders weiche Pitch-/Read-Transitions werden konkretisiert.

## Impact

- Betrifft `LivingFreezeEngine` und die zugehörigen JUCE-Unit-Tests.
- Keine neuen UI- oder Host-Parameter und keine Änderung an Parameter-State-Formaten.
- Keine neuen Abhängigkeiten; zusätzliche Berechnungen bleiben echtzeitsicher und allocation-frei im Audio-Callback.
