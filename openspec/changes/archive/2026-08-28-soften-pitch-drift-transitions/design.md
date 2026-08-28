## Context

Die aktuelle `LivingFreezeEngine` verwendet eine gemeinsame Drift-Glättung für Position, Stereo und Playback-Speed. Die Voice-Ziele werden nicht-periodisch aktualisiert, aber Playback-Speed wird dabei mit derselben kurzen Zeitkonstante wie die übrigen Bewegungen angenähert. Die bestehenden Safe-Read-Berechnungen und envelope-basierten Voice-Restarts sind Click-Safety-relevant und bleiben unangetastet. Siehe `proposal.md` und die Delta-Spec für Motivation und Verhalten.

## Goals / Non-Goals

**Goals:**

- Eine zentrale, testbare interne Pitch-Drift-Kurve einführen, deren Wert bei `0.0` null und bei `1.0` eins ist und die im unteren Bereich sublinear verläuft.
- Position, Stereo und Pitch mit getrennten Amounts und geeigneten Glättungskoeffizienten behandeln.
- Playback-Speed-Zielwerte und Drift-Parameteränderungen kontinuierlich und innerhalb des bestehenden Speed-Limits verarbeiten.
- Die vorhandene nicht-periodische Voice-Unabhängigkeit, Safe-Read-Logik und Echtzeitfähigkeit bewahren.
- JUCE-Tests für Mapping, Null-/Niedrig-/Hoch-Drift, kontinuierliche Speed-Änderungen und lange Renderstrecken ergänzen.

**Non-Goals:**

- Keine neuen UI- oder Host-Parameter.
- Keine Änderung der Capture-Dauer, Window-Envelope, Voice-Anzahl, Dry/Wet-Struktur oder Click-Safety-Architektur.
- Keine Änderung des maximalen konfigurierten Playback-Speed-Bereichs ohne klangliche Notwendigkeit.

## Decisions

### Separate internal modulation amounts

Der normalisierte Parameterwert bleibt die einzige sichtbare Eingabe. Intern wird daraus ein früher reagierender Positions-Amount, ein früher reagierender Stereo-Amount und ein Pitch-Amount über eine sublineare Kurve abgeleitet. Position und Stereo verwenden weiterhin ihre bestehenden Reserven und Grenzen; nur die Pitch-Zielabweichung wird durch den Pitch-Amount skaliert. Dadurch wird die gewünschte Priorisierung erreicht, ohne Parameter- oder State-Kompatibilität zu ändern.

Als Kurvenfamilie wird zunächst eine Potenz-/Smoothstep-artige Funktion mit expliziten Endpunkten verwendet; die konkrete Exponentwahl bleibt als interne Konstante abstimmbar. Eine lineare Kurve würde den unteren Bereich zu früh hörbar machen, eine harte Piecewise-Schwelle könnte beim Überschreiten der Schwelle selbst eine Modulationskante erzeugen.

### Dedicated pitch smoothing

Die Voice erhält einen separaten Playback-Speed-Smoothing-Koeffizienten, berechnet aus Sample-Rate und einer nominalen Zeit von etwa 300 ms. Der vorhandene kürzere Koeffizient bleibt für Position und Stereo zuständig. Ziele werden weiterhin nur an den bestehenden nicht-periodischen Update-Zeitpunkten neu gewürfelt; die laufende Annäherung erfolgt sampleweise. Bei Drift `0` werden Ziele und effektive Geschwindigkeit deterministisch auf `1.0` geführt.

Eine `juce::SmoothedValue` ist nicht erforderlich: Ein vorberechneter exponentieller Koeffizient vermeidet zusätzliche Zustands- und Reset-Komplexität und ist im Audio-Callback allocation-frei. Der Koeffizient wird in `prepare` vorbereitet.

### Safe-read interaction

Die Safe-Read-Reservierung behält den maximalen Speed-Faktor, damit auch bei niedrigerem aktuellem Pitch-Amount spätere hohe Drift-Ziele sicher geplant sind. Position-/Stereo-Zielwerte werden weiterhin gegen die aktuellen sicheren Grenzen geklemmt. Playback-Speed wird nach jeder Annäherung nochmals auf den bestehenden Bereich begrenzt; dadurch kann die langsamere Glättung keine ungültigen Read-Positionen erzeugen.

### Verification strategy

Die Unit-Tests werden um beobachtbare Messungen ergänzt. Für Mapping und Range wird eine kleine öffentliche oder testgeeignet zugängliche reine Hilfsfunktion bevorzugt, statt private Voice-Felder freizulegen. Für Smoothing und Drift-Parameterwechsel werden Engine-Renderblöcke ausgewertet: aufeinanderfolgende Speed-/Output-Bewegungen dürfen keine direkten Zielsprünge oder nicht-finite Werte zeigen, während Drift `1.0` weiterhin stärker vom Drift-Nullzustand abweicht. Die vorhandenen High-Drift-Boundary- und Long-Render-Tests bleiben bestehen.

## Risks / Trade-offs

- [Pitch bleibt bei mittlerem Drift zu subtil] → Kurvenform und maximale Abweichung gemeinsam anhand der Low-/High-Range-Tests abstimmen; Endpunkt bei Drift `1.0` unverändert lassen.
- [300 ms reagiert zu träge auf neue Ziele] → Update-Intervall und Smoothing-Zeit getrennt betrachten und die Zeit innerhalb des spezifizierten Bereichs justieren, ohne direkte Zielzuweisung einzuführen.
- [Drift-Sicherheitsreserve ist für niedrige Werte unnötig groß] → Reserve zunächst am maximal möglichen Speed-/Positionsfall ausrichten, da dies die bestehende Sicherheit schützt; erst nach Messung optimieren.
- [Tests messen nur Ausgangssamples statt interne Speed-Werte] → deterministische Testsignale und kurze, gezielte Renderfenster verwenden; falls erforderlich eine minimale testgeeignete Diagnose-Schnittstelle ergänzen, ohne Produktionsparameter zu exponieren.

## Migration Plan

Keine Daten- oder Preset-Migration erforderlich. Implementierung, Tests und Validierung erfolgen innerhalb der bestehenden Engine. Bei klanglich unbefriedigendem Ergebnis kann die Änderung durch Zurücksetzen der internen Kurven-/Zeitkonstanten rückgängig gemacht werden; UI- und State-Formate bleiben kompatibel.
