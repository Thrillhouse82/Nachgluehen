## 1. Output-Gain-Mapping und finales Staging

- [x] 1.1 Den Output-Gain-Parameter auf eine monotone, inverse asymmetrische Normalisierung mit Mute bei 0.0, 0 dB bei 0.5 und +12 dB bei 1.0 umstellen sowie dB-Textdarstellung und State-Restore absichern.
- [x] 1.2 Sicherstellen, dass die finale geglättete Output-Gain-Stufe bei 0 dB exakt Unity (1.0) bleibt und Mute, +6 dB und +12 dB weiterhin korrekt und endlich verarbeitet.
- [x] 1.3 Die bestehende Post-Gain-Clip-Erkennung und ihren lock-freien Hold gegen das neue Parameter-Mapping regressionssicher prüfen.

## 2. Gleichwertiges Output-Layout

- [x] 2.1 Das feste Editor-Layout so aufteilen, dass Drift, Dry/Wet und Output identische Rotary-Slider-Bounds und Kreisgrößen erhalten.
- [x] 2.2 Für Output-Label und roten `CLIP`-Status unabhängige, sichtbare Bounds vergeben und die Anzeige ausschließlich vom Clip-Hold-Status steuern.

## 3. Texture-Unity-Level

- [x] 3.1 Die deterministische Wet-Texture-Signalkette hinsichtlich Voice-Summierung, Window-Envelopes und bestehender Gain-Kompensation messen und den systematischen Pegelverlust identifizieren.
- [x] 3.2 Die minimale konservative interne Gain-Kompensation oder Mischkalibrierung implementieren, die bei 100 % Wet und 0 dB Output einen kontrollierten musikalisch brauchbaren Pegel liefert, ohne Envelope-Form, Click-Safety oder Peak-Sicherheit zu verletzen.

## 4. Tests und Validierung

- [x] 4.1 JUCE-Tests für Output-Gain-Mapping (Minimum, Mittelpunkt, Maximum, Roundtrip), dB-Text, Host-Automation und State-Restore ergänzen.
- [x] 4.2 JUCE-Tests für Unity bei 0 dB, Mute, +6 dB, +12 dB, finite geglättete Übergänge sowie Post-Gain-Clip-Hold und No-Clip ergänzen.
- [x] 4.3 Einen deterministischen Wet-Texture-Level-Regressionstest ergänzen, der offensichtlichen systematischen Pegelverlust bei 100 % Wet und 0 dB Output erkennt.
- [x] 4.4 Projekt bauen, vollständige JUCE-Test-Suite und strikte OpenSpec-Validierung ausführen sowie die UI- und Pegel-Akzeptanz in Ableton manuell prüfen.
