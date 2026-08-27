## Context

Siehe `proposal.md` für die Motivation und `specs/living-freeze/spec.md` für den Verhaltensvertrag. Die bestehende Engine hält acht vorallokierte Voices, liest mit linearer Interpolation aus einem eingefrorenen Vektor und wickelt Positionen in `readLinear()` zyklisch um. Voice-Starts und Neustarts werden derzeit über die gesamte Capture-Länge verteilt; Position, Playback-Speed und Stereo-Offset werden während des laufenden Windows geglättet. Die aktuelle Wet-Mischung normalisiert die gewichtete Summe durch die momentane Summe der Window-Envelopes.

Die Implementierung muss im Audio-Callback allokationsfrei und deterministisch genug für die bestehenden Seed-Tests bleiben. Sichtbare Parameter und die Capture-Dauer bleiben unverändert.

## Goals / Non-Goals

**Goals:**

- Für jede Voice einen sicheren, kontinuierlich lesbaren Bereich einschließlich maximaler Geschwindigkeits- und Offset-Reserve bestimmen.
- Voice-Positionen und Drift-Ziele innerhalb dieses Bereichs halten und Neustarts ausschließlich bei praktisch stummem Envelope durchführen.
- Die bestehende weiche Window-Funktion beibehalten bzw. an den Endpunkten explizit auf near-zero absichern.
- Die Envelope als tatsächlichen Voice-Gain erhalten und Überlappungspegel mit einer stabilen, nicht sampleweise envelope-invertierenden Strategie kontrollieren.
- Regressionstests für Grenzen, Neustarts, Envelopes, Mischung, Drift und Langzeit-Finitheit ergänzen.

**Non-Goals:**

- Keine neuen Plugin-Parameter oder Änderungen an der sichtbaren Bedienoberfläche.
- Keine Änderung der Capture-Dauer, Voice-Anzahl oder des grundlegenden nicht-periodischen Texture-Konzepts, sofern dies für die Sicherheit nicht erforderlich ist.
- Keine zusätzlichen Effekte wie Delay, Reverb oder Transient Detection.

## Decisions

### Sichere Read-Region statt hörbares Buffer-Wrapping

Die maximal benötigte Lesestrecke wird aus Window-Länge, maximaler Playback-Speed und zulässigem Positions-/Stereo-Offset abgeleitet. Daraus wird eine obere sichere Startposition kleiner als `capturedLength - 1` berechnet; alle initialen und zufällig gewählten Starts werden in `[0, safeStartMax]` begrenzt. Die laufende Position wird vor dem Lesen zusätzlich in die für die aktuelle Voice zulässige Region geclippt. `readLinear()` darf weiterhin als allgemeine Hilfsfunktion zyklisch adressieren, aber die Texture-Voice darf diese Wrap-Eigenschaft innerhalb eines hörbaren Windows nicht mehr benötigen.

Als Sicherheitsreserve wird mindestens die Interpolationsnachbarposition sowie eine Reserve für den maximalen Drift- und Speed-Fehler berücksichtigt. Die sichere Region wird bei jeder Änderung der effektiven Window-/Speed-Grenzen neu angewendet; dadurch bleiben auch hohe Driftwerte innerhalb des Bereichs.

Alternative: Jede Voice könnte auf einen separat kopierten, verlängerten Buffer zugreifen. Das würde zusätzliche Speicher- und Capture-Komplexität erzeugen und die eigentliche Grenze nur verstecken; Clamping auf einen kontinuierlichen Bereich ist für das MVP einfacher und echtzeitsicher.

### Neustart nur am Window-Ende bei near-zero Gain

Die Voice schreitet bis zum Ende ihres geplanten sicheren Fensters fort. Sobald die Phase das Ende erreicht, wird die Position nicht während eines hörbaren Samples umgeschaltet; der Neustart wird an die bestehende near-zero-Envelope-Phase gekoppelt. Der neue Start wird vor dem nächsten Einblenden gesetzt, Phase und Read-Position werden auf gültige Werte zurückgesetzt, und der nächste Zyklus beginnt bei near-zero Gain.

Falls Drift die aktuelle Zielposition außerhalb der sicheren Region führen würde, wird nur das Ziel begrenzt und die laufende Position weich dorthin bewegt. Ein harter Positionswechsel ist ausschließlich im stummen Restart-Zustand zulässig.

### Windowing und stabile Mischung

Die bestehende Raised-Cosine/Sinus-Hüllkurve bleibt die Envelope, da sie an Phase 0 und 1 exakt null liefert und im Inneren nicht negativ ist. Die Render-Summe bleibt `sample * envelope` pro Voice. Die Division durch die momentane Envelope-Summe wird entfernt und durch eine konstante Voice-Gain-Skalierung ersetzt, die auf der festen maximalen Voice-Anzahl bzw. erwarteten Überlappung basiert. Falls zur Pegelstabilität erforderlich, wird nur ein langsam geglätteter Kompensationsfaktor verwendet, der aus einer vorallokierten Zustandsvariable gespeist wird; er darf kein einzelnes Window auf Unity-Gain zurückskalieren.

Alternative: Eine geglättete Division durch die Envelope-Summe würde die Pegelschwankung stärker reduzieren, kann aber weiterhin den Fade eines einzelnen Windows aufheben. Die feste Kompensation erfüllt die Priorität der wirksamen Envelopes mit vorhersehbarem CPU-Aufwand.

### Drift-Clamping ohne neue Modulationssprünge

PositionTarget, speedTarget und stereoTarget werden weiterhin deterministisch aus dem bestehenden RNG erzeugt und geglättet. Vor der Zuweisung werden Positions- und Stereoanteile so begrenzt, dass die kombinierte Leseposition innerhalb der sicheren Region bleibt. Playback-Speed wird auf einen positiven, unterstützten Bereich begrenzt und bei der Berechnung der maximalen Lesestrecke berücksichtigt. Die bestehende langsame Target-Interpolation bleibt bestehen.

### Teststrategie

Die Tests bleiben im JUCE-Unit-Test-Programm. Zusätzlich zu den bestehenden Tests werden ein Capture mit stark unterschiedlichen Randwerten, explizite Window-Endpunkt-/Monotonieprüfungen, ein einzelnes Window mit reduzierter Envelope-Amplitude, Restart-/Boundary-Messungen und ein langer High-Drift-Render mit maximalem Sample-Step ergänzt. Tests greifen nach Möglichkeit über beobachtbares Audioverhalten zu; interne Hilfsfunktionen werden nur dann sichtbar gemacht, wenn dies für die Window-Vertragsprüfung ohne Produktions-API-Ausweitung sinnvoll ist.

## Risks / Trade-offs

- [Weniger nutzbare Capture-Länge] → Sicherheitsreserve nur so groß wie für maximale Speed, Drift und Interpolation nötig wählen; Startpositionen weiterhin deterministisch verteilt und zwischen Zyklen variiert wählen.
- [Niedrigerer Wet-Pegel durch feste Kompensation] → Kompensationsfaktor anhand der acht Voices und der erwarteten Überlappung kalibrieren und mit bestehenden Dichte-/Pegeltests prüfen.
- [Drift wirkt bei kurzen Captures schwächer] → sichere Region dynamisch aus der tatsächlichen Capture-Länge berechnen und innerhalb dieser Region den gesamten erlaubten Driftbereich nutzen.
- [Restart-Zeitpunkt kann bei unpassendem Clamping hörbar werden] → Position nur am Window-Ende bei near-zero Gain setzen und vor dem Lesen auf gültige Grenzen prüfen.
- [Neue Grenzlogik erhöht die Rechenarbeit pro Sample] → nur einfache `jlimit`-/arithmetische Operationen und vorab berechnete Konstanten im Callback verwenden.

## Migration Plan

1. Delta-Spezifikation und Design reviewen.
2. Voice-State und Render-Mischung intern anpassen, ohne Parameter- oder State-Formatänderung.
3. Bestehende und neue JUCE-Tests ausführen; bei Bedarf Sicherheitsreserve und feste Kompensation anhand der Messwerte feinjustieren.
4. Bei Regressionen kann die Änderung als einzelner Engine-Commit zurückgenommen werden; gespeicherte Plugin-Zustände bleiben kompatibel, da keine sichtbaren Parameter geändert werden.

## Open Questions

Keine.
