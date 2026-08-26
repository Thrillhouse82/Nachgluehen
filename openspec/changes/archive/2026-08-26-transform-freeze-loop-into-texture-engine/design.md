## Context

`LivingFreezeEngine` hält bereits einen begrenzten Stereo-Capture und mischt ein eingefrorenes Signal über geglättetes Freeze-Gain und Dry/Wet zurück. Die aktuelle Wiedergabe verwendet jedoch eine gemeinsame `playbackPosition` und liest den gesamten Capture zyklisch. Die bestehende Engine arbeitet im Audio-Callback; Capture- und Playback-Zustände müssen daher vorab reserviert sein und alle Modulationen müssen numerisch sicher bleiben.

## Goals / Non-Goals

**Goals:**

- Den Capture als Material-Pool für eine kontinuierliche, überlappende Texture-Wiedergabe nutzen.
- Einen festen kleinen Voice-Pool mit weichen Window-Envelopes und unabhängigen Zuständen einführen.
- Bei Drift null eine stabile, nicht-klassische Texture liefern und bei positivem Drift deterministische, geglättete Bewegung ermöglichen.
- Freeze-Übergänge, Voice-Wechsel und Buffer-Adressierung klickfrei und echtzeitsicher halten.
- Die bestehende Capture-, Parameter- und Plugin-Schnittstelle unverändert lassen.

**Non-Goals:**

- Keine neue Transienten-Erkennung oder sichtbare Bedienparameter.
- Keine Erweiterung um Granular-Synthese, Delay, Reverb, Reverse, Tempo-Synchronisation oder variable Capture-Länge.
- Keine Heap-Allokation, Sperre oder externe Ressourcen im Audio-Callback.

## Decisions

### Fester Voice-Pool

Die Engine erhält eine feste `std::array` von ungefähr 8--12 Stimmen (Ausgangspunkt: 8), wobei jede Stimme Read-Position, Geschwindigkeit, Window-Phase, Window-Länge, Start-/Endzustand und optional geglättete Driftwerte hält. Der Pool wird in `prepare` initialisiert und in `reset` zurückgesetzt. Eine feste, moderate Voice-Anzahl begrenzt CPU-Kosten; ein einzelner Read-Head wäre klanglich nicht ausreichend.

### Überlappende Window-Planung

Die Stimmen werden in einem stabilen zeitlichen Raster versetzt gestartet, mit mehreren gleichzeitig aktiven Fenstern. Ein Hann- oder Raised-Cosine-Fenster wird pro Stimme aus der Phase berechnet. Die Fensterlänge bleibt lang genug, um keinen granularen Charakter zu erzwingen, aber kurz genug, um nicht den gesamten Capture als wiederkehrende Einheit zu präsentieren. Startpositionen werden innerhalb des Captures verteilt und beim erneuten Einsatz nicht pauschal auf denselben Buffer-Anfang gesetzt.

Die Stimmen werden in jedem Sample mit linearer Interpolation aus dem Capture gelesen. Beim Ende eines Fensters wird die Stimme weich beendet und für einen neuen, überlappenden Einsatz reinitialisiert; der Audio-Callback schreibt dabei nur in bereits vorhandene Zustände.

### Verhalten bei Drift 0 %

Drift null deaktiviert Zufallsupdates und zusätzliche Geschwindigkeits-, Positions- oder Stereo-Modulation. Die gestaffelten Stimmen, ihre festen Startpositionen und Window-Phasen bleiben trotzdem aktiv. So entsteht ein vorhersehbarer, materialtreuer Grundzustand, der nicht auf einen vollständigen Capture-Zyklus als einzelne Stimme zurückfällt.

### Geglättete deterministische Drift

Bei positivem Drift erzeugt der vorhandene Seed-basierte Zufall neue Zielwerte in unregelmäßigen Zeitabständen. Zielwerte werden langsam geglättet und auf sichere Bereiche begrenzt. Diese Modulation darf pro Stimme Position, Geschwindigkeit, Window-Länge, Einsatzabstand und leichte Stereo-Abweichungen beeinflussen; die Grundwerte bleiben bei niedriger Intensität nahe am materialtreuen Zustand. Ein einfacher periodischer LFO wird nicht eingeführt.

### Übergänge und Pegel

Die vorhandene Freeze-Gain- und Dry/Wet-Glättung bleibt die äußere Übergangsschicht. Zusätzlich startet jede Voice mit Envelope-Ramp und wird beim Deaktivieren von Freeze über den globalen Übergang bzw. einen Voice-Fade aus dem Signal genommen. Die Fenster werden auf einen definierten Pegel normalisiert, damit die Überlappung nicht zu einem unnötigen Pegelsprung führt.

### Capture- und Stereo-Kompatibilität

Der bestehende Recent-Audio-Capture bleibt unverändert. Die beiden Capture-Kanäle werden separat gelesen; optionale Voice-Stereo-Abweichungen werden als kleine, begrenzte Kanalgewichtung umgesetzt und dürfen bei Drift null nicht zufällig variieren. Die vorhandene Ring- und Frozen-Buffer-Kapazität bleibt ausreichend für die feste Capture-Dauer.

### Tests als Verhaltensnachweis

Die JUCE-Tests prüfen die Engine über viele Capture-Längen: kontinuierliche Pegelversorgung und endliche Samples, maximale Sprünge an Voice-Grenzen, Impulspositionen gegen exakte `capturedLength`-Wiederholung, stabile identische Ausgaben bei gleichem Seed und Drift null sowie zunehmende, deterministische Abweichung bei höheren Drift-Werten. Die Tests bleiben signalbasiert und machen keine subjektive Klangbewertung.

## Risks / Trade-offs

- [Mehrere Fenster verwischen Transienten] → Voice-Anzahl und Window-Länge konservativ starten; bei Drift null feste, materialnahe Werte verwenden.
- [Zu lange Fenster behalten Loop-Charakter] → versetzte Starts und nicht identische Fensterzyklen testen; die Wiederholung des kompletten Captures darf nicht der einzige periodische Vorgang sein.
- [Stimmen erhöhen CPU-Kosten] → kleiner fester Pool, einfache lineare Interpolation und keine zusätzliche spektrale Verarbeitung.
- [Kurze Captures bieten wenig Material] → bestehende Capture-Dauer beibehalten und Fenster innerhalb des verfügbaren Bereichs begrenzen; bei sehr kurzen Captures sichere Fallbacks verwenden.
- [Drift erzeugt Klicks oder instabile Werte] → alle Zielwerte begrenzen, kontinuierlich glätten und Finite-Checks am Ausgang beibehalten.

## Migration Plan

1. Voice-Zustände und Window-Sampler in der bestehenden Engine ergänzen.
2. Die bisherige einzelne Playback-Position durch den Voice-Pool ersetzen, ohne öffentliche Parameter zu ändern.
3. Neue und bestehende JUCE-Tests ausführen und Grenzfälle für kurze bzw. leere Captures prüfen.
4. Bei Bedarf nur interne feste Voice-/Window-Konstanten abstimmen; bei Regression kann die Änderung per Commit-Rollback auf die bisherige Engine zurückgesetzt werden.
