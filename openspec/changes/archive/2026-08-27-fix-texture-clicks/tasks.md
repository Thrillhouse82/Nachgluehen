## 1. Sichere Voice-Lesebereiche

- [x] 1.1 Sichere Lesestrecke aus tatsächlicher Capture-Länge, Window-Länge, maximaler Playback-Speed, Interpolationsnachbar und maximalen Drift-/Stereo-Offsets ableiten.
- [x] 1.2 Voice-Startpositionen bei Initialisierung und Neustart innerhalb der sicheren Startregion verteilen und zyklisches Wrapping während eines hörbaren Windows vermeiden.
- [x] 1.3 Read-Positionen und Drift-Ziele auf gültige sichere Grenzen begrenzen, einschließlich positiver Playback-Speed und kombinierter Positions-/Stereo-Offsets.

## 2. Klickfreie Voice-Zyklen und Mischung

- [x] 2.1 Voice-Restarts an das near-zero-Ende der Window-Envelope koppeln und sicherstellen, dass der neue Zyklus bei near-zero Gain beginnt.
- [x] 2.2 Window-Auswertung auf nicht-negative kontinuierliche Endpunktwerte prüfen und die bestehende weiche Fade-In-/Fade-Out-Form ohne harte Übergänge erhalten.
- [x] 2.3 Envelope-Summen-Normalisierung aus der Texture-Mischung entfernen und durch eine konstante oder langsam geglättete Voice-Gain-Kompensation ersetzen, die einzelne Fades nicht neutralisiert.
- [x] 2.4 Freeze-, Drift- und Voice-Zustandsübergänge auf finite Werte und allokationsfreie Callback-Ausführung prüfen.

## 3. Regressionstests

- [x] 3.1 Buffer-Boundary-Test mit stark unterschiedlichen Anfangs-/Endwerten ergänzen und Sample-Sprünge bei hoher Window-Amplitude prüfen.
- [x] 3.2 Voice-Restart-Test ergänzen, der near-zero Gain vor Positionswechsel und near-zero Gain am Start eines neuen Zyklus absichert.
- [x] 3.3 Window-Tests für Endpunkte, Nichtnegativität und kontinuierlichen Verlauf ergänzen.
- [x] 3.4 Normalisierungstests für wirksames Single-Voice-Fading und glatten Wechsel der Voice-Überlappung ergänzen.
- [x] 3.5 High-Drift-Boundary-Test für sichere Positionen, Offsets und maximale Sample-Sprünge ergänzen.
- [x] 3.6 Langzeit-Render über viele Voice-Zyklen mit NaN-/Infinity-, Sample-Step- und ausreichender Texturdichte-Prüfung ergänzen.

## 4. Verifikation

- [x] 4.1 Bestehende und neue JUCE-Unit-Tests ausführen und Grenzwerte für Sicherheitsreserve und Mischpegel anhand der Ergebnisse kalibrieren.
- [x] 4.2 Build und vollständigen Testlauf ausführen; sicherstellen, dass keine sichtbaren Parameter, State-Kompatibilität oder Echtzeitrestriktionen regressieren.
