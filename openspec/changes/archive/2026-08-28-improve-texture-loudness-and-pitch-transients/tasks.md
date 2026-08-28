## 1. Gain-Mischung analysieren und anpassen

- [x] 1.1 Bestehende Voice-Summierung und `textureGainCompensation` anhand korrelierter, decorrelierter und stiller/kurzer Envelope-Abschnitte vermessen.
- [x] 1.2 Eine energiebezogene Voice-Gain-Berechnung mit geglättetem Verlauf, konservativem Ceiling und Mindestpegel implementieren; direkte Envelope-Summen-Normalisierung darf nicht die alleinige Lautheitsgrundlage sein.
- [x] 1.3 Sicherstellen, dass die neue Gain-Regelung bei Window-Restarts und variabler Voice-Anzahl stabil bleibt, finite Werte erzeugt und keine unbegrenzte Verstärkung ermöglicht.

## 2. Pitch- und Read-Transitions beruhigen

- [x] 2.1 Playback-Speed-Ziele von unabhängigen Vollbereichs-Zufallswerten auf bounded Random-Walk-Schritte pro Voice umstellen.
- [x] 2.2 Schrittweite und Pitch-Smoothing so abstimmen, dass niedrige/mittlere Drift subtil bleiben, hohe Drift den vorgesehenen Bereich über mehrere Zielwechsel erreichen kann und keine abrupten Pitch-Impulse entstehen.
- [x] 2.3 Drift-abhängige Positions- und Stereo-Grenzen ohne direkte hörbare Read-Position-Sprünge anwenden; laufende Offsets kontinuierlich an neue sichere Ziele führen.
- [x] 2.4 Bestehende Safe-Read-, Window-Envelope-, Near-Zero-Restart- und Click-Safety-Logik gegen Regressionen prüfen.

## 3. Regressionstests erweitern

- [x] 3.1 RMS- und Peak-Tests für stabile Freeze-Texturen mit korrelierten und decorrelierten Voices bei Drift 0, 0.2 und 1.0 ergänzen.
- [x] 3.2 Tests für Gain-Floor, Gain-Ceiling, wiederholte Voice-Restarts und finite Output-Werte ergänzen.
- [x] 3.3 Tests für Random-Walk-Zielschritte, maximale Speed-Änderungen und kontinuierliche Speed-/Output-Verläufe bei Drift- und Zielwechseln ergänzen.
- [x] 3.4 Einen Test für Drift-Parameteränderungen während aktivem Freeze ergänzen, der Read-Position und Output auf Sprungfreiheit prüft.
- [x] 3.5 Bestehende Boundary-, Determinismus-, Long-Render- und Click-Safety-Tests ausführen und bei Bedarf schärfen.

## 4. Validierung

- [x] 4.1 Projekt bauen und die vollständige JUCE-Test-Suite erfolgreich ausführen.
- [x] 4.2 OpenSpec-Change strikt validieren und Änderungen auf die vorgesehenen DSP-, Test- und Spec-Dateien begrenzen.
