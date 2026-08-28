## Why

Bei mittleren und höheren Drift-Werten erzeugen Änderungen der Playback-Speed derzeit teilweise harte, transientenartige Pitch-Übergänge. Die bestehende Texture Engine soll ihre Click-Safety und flächige Mehrstimmigkeit behalten, während Pitch-Drift bei niedrigen Werten subtiler einsetzt und sich bei allen Werten musikalisch weich bewegt.

## What Changes

- Die interne Drift-Modulation wird in Positions-, Stereo- und Pitch-/Playback-Speed-Anteile getrennt.
- Pitch-Drift wird über ein nichtlineares Mapping aus dem sichtbaren Drift-Wert abgeleitet, sodass niedrige und mittlere Werte deutlich weniger Pitch-Abweichung erzeugen.
- Playback-Speed erhält eine eigene, längere Glättung für kontinuierliche Zielwechsel und Änderungen des Drift-Parameters während aktivem Freeze.
- Hohe Drift-Werte behalten grundsätzlich den bestehenden maximalen Pitch-/Speed-Spielraum und die Möglichkeit zu stärker auseinanderlaufenden Voices.
- Drift 0 deaktiviert absichtliche Pitch-Modulation und hält Playback-Speed effektiv bei `1.0`, ohne die Texture Engine zu deaktivieren.
- Bestehende sichere Read-Regionen, Voice-Restarts, Window-Übergänge und Echtzeitvorgaben bleiben erhalten.
- Die sichtbaren Parameter bleiben auf Freeze, Drift und Dry/Wet beschränkt.

## Capabilities

### New Capabilities

Keine.

### Modified Capabilities

- `living-freeze`: Anforderungen an Drift-Modulation werden um getrennte Pitch-Skalierung, langsame Playback-Speed-Glättung und kontinuierliche Pitch-Zielwechsel konkretisiert.

## Impact

- Betrifft die DSP-Implementierung der Living-Freeze-/Texture-Voice-Modulation und deren JUCE-Unit-Tests.
- Keine Änderungen an UI-Parametern, Parameter-Namen oder State-Serialisierung.
- Keine neuen Laufzeitabhängigkeiten; Mapping und Smoothing müssen allocation-frei und echtzeitsicher im Audio-Callback bleiben.
