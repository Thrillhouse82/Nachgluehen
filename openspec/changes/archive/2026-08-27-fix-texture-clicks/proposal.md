## Why

Die aktuelle Texture-Engine kann bei mehreren überlappenden Voices hörbare Knackser erzeugen, wenn ein Read-Head während eines hörbaren Windows zyklisch vom Ende des Freeze-Buffers an dessen Anfang springt. Zusätzlich kann die sampleweise Normalisierung anhand der Envelope-Summe das gewünschte Fade-In/Fade-Out einzelner Voices teilweise neutralisieren.

Der Change soll interne Read-Head- und Voice-Übergänge sicher und klickfrei machen, ohne den kontinuierlichen, nicht-loopartigen Charakter der Texture-Engine oder die musikalische Wirkung von Drift zu verlieren.

## What Changes

- Texture-Voices verwenden nur sichere Read-Bereiche, deren geplante Lesestrecke einschließlich Playback-Speed-, Drift- und Stereo-Offsets vollständig innerhalb des Freeze-Buffers liegt.
- Voice-Startpositionen und Drift-Ziele werden auf diese sicheren Bereiche begrenzt; ein Read-Head wird nur bei praktisch stummem Envelope-Gain neu positioniert.
- Window-Envelopes behalten einen kontinuierlichen Fade-In und Fade-Out mit Gain nahe null an Anfang und Ende.
- Die Output-Mischung verwendet eine stabile, geglättete Gain-Kompensation, die Überlappungen beherrscht, aber das Window-Fading nicht durch Division durch die aktuelle Envelope-Summe neutralisiert.
- Bestehendes weiches, kontinuierliches Drift-Verhalten und die Echtzeitsicherheitsanforderungen bleiben erhalten.
- JUCE-Unit-Tests decken Buffer-Grenzen, Voice-Restarts, Window-Verlauf, Normalisierung, Drift-Grenzen und Langzeitstabilität ab.

## Capabilities

### New Capabilities

Keine.

### Modified Capabilities

- `living-freeze`: Read-Head-Grenzen, klickfreie Voice-Neustarts, wirksames Window-Fading, stabile Output-Mischung und sichere Drift-Bewegung werden als explizite Anforderungen präzisiert.

## Impact

- Betroffen sind die interne Texture-Voice-/Read-Head-Steuerung, Window-Envelope-Auswertung, Wet-Signal-Mischung und Drift-Begrenzung.
- Betroffen sind die bestehenden JUCE-Unit-Tests und neue Regressionstests für die Texture-Engine.
- Es werden keine sichtbaren Parameter, Plugin-APIs oder externen Abhängigkeiten geändert.
- Die Audio-Callback-Anforderungen bleiben unverändert: keine Allokationen, Locks, Dateizugriffe oder blockierenden Operationen.
