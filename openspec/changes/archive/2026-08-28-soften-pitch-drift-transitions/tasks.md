## 1. Drift-Modell vorbereiten

- [x] 1.1 Bestehende Living-Freeze-Voice- und Drift-Zustände prüfen und interne Konstanten für getrennte Positions-, Stereo- und Pitch-Amounts sowie die Pitch-Smoothing-Zeit definieren.
- [x] 1.2 Eine allocation-freie, deterministische Pitch-Drift-Mapping-Funktion mit `0.0 -> 0.0`, `1.0 -> 1.0` und deutlich sublinearem Verhalten bei `0.2` implementieren und testgeeignet zugänglich machen.

## 2. Texture-Voice-Modulation anpassen

- [x] 2.1 Position- und Stereo-Ziele weiterhin früh aus Drift ableiten, aber ihre Amounts von der Pitch-Skalierung entkoppeln.
- [x] 2.2 Playback-Speed-Ziele pro Voice mit dem nichtlinearen Pitch-Amount skalieren, bei Drift 0 auf `1.0` setzen und den bestehenden maximalen Speed-Bereich bei Drift 1.0 erhalten.
- [x] 2.3 Einen separaten langsameren Playback-Speed-Smoothing-Koeffizienten in `prepare` berechnen und ausschließlich für Speed-Annäherungen verwenden; Position und Stereo behalten die kürzere Glättung.
- [x] 2.4 Änderungen des Drift-Parameters und nicht-periodische Voice-Zielwechsel kontinuierlich verarbeiten, inklusive erneuter Begrenzung auf Speed- und Safe-Read-Grenzen.
- [x] 2.5 Bestehende Window-Envelopes, Near-Zero-Voice-Restarts, Safe-Read-Reservierungen und Echtzeitvorgaben unverändert absichern.

## 3. JUCE-Tests ergänzen

- [x] 3.1 Pitch-Drift-Zero- und Nonlinear-Mapping-Tests für `0.0`, `0.2` und `1.0` hinzufügen.
- [x] 3.2 Low- und High-Drift-Range-Tests ergänzen: niedrige Drift-Werte bleiben pitch-subtil, während Drift 1.0 den vorgesehenen maximalen Pitch-/Speed-Bereich weiterhin nutzen kann.
- [x] 3.3 Tests für neue Pitch-Ziele und Drift-Parameteränderungen während Freeze ergänzen, die keine direkten Speed-/Output-Sprünge und nur kontinuierliche Annäherung erlauben.
- [x] 3.4 Einen Long-Render-Test über viele nicht-periodische Zielwechsel ergänzen bzw. erweitern, der Speed-/Output-Grenzen, Finite-Werte und Click-Safety prüft.
- [x] 3.5 Bestehende Click-Safety-, Determinismus-, Boundary- und Texture-Continuity-Tests ausführen und bei Bedarf so schärfen, dass die bisherige Sicherheit nachweisbar erhalten bleibt.

## 4. Validierung

- [x] 4.1 Projekt bauen und alle JUCE-Unit-Tests erfolgreich ausführen.
- [x] 4.2 OpenSpec-Change validieren und sicherstellen, dass nur Planungsartefakte dieses Changes sowie die vorgesehenen DSP-/Teständerungen betroffen sind.
