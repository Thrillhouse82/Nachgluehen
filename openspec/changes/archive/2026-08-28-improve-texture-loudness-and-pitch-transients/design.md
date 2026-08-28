## Context

Die aktuelle `LivingFreezeEngine` summiert acht envelope-gegewichtete Voices und kompensiert den Pegel im Wesentlichen mit `1 / envelopeSum`. Das bewahrt bei korrelierten Voices den Spitzenpegel, attenuiert aber bei kleinen Positions-, Stereo- oder Pitch-Unterschieden die wahrgenommene Energie. Außerdem werden Pitch-Ziele weiterhin als neue zufällige absolute Werte gewählt; selbst ein geglätteter Übergang kann dadurch als deutliches „WUP“ hörbar bleiben. Drift-Änderungen können zusätzlich einen laufenden Read-Offset an neue Sicherheitsgrenzen klemmen.

## Goals / Non-Goals

**Goals:**

- Eine energiebezogene, begrenzte Voice-Mischung erreichen, die bei decorrelierten Voices nicht unnötig leise wird und an Window-Grenzen ruhig bleibt.
- Pitch-Ziele als kleine, begrenzte Schritte eines langsamen nicht-periodischen Random-Walks fortschreiben.
- Pitch-Smoothing und Zielschrittweite so abstimmen, dass hohe Drift weiterhin hörbar bleibt, aber keine abrupten Pitch-Impulse erzeugt.
- Drift-bedingte Offset-/Safety-Anpassungen kontinuierlich behandeln und bestehende sichere Read-Bereiche einhalten.
- Messbare Tests für RMS-Pegel, Gain-Grenzen, Speed-/Positionskontinuität und lange Renderstrecken ergänzen.

**Non-Goals:**

- Keine neue UI-Steuerung für Gain, Pitch, Smoothing oder Voice-Anzahl.
- Keine Änderung an Capture-Dauer, Window-Form, Dry/Wet-Parameter oder Host-State-Format.
- Keine vollständige Lautheitsnormalisierung gegen ein externes LUFS-Ziel und kein Ersatz der Texture Engine durch einen Limiter oder Kompressor.

## Decisions

### Energy-aware voice mixing

Die aktuelle Envelope-Summe wird durch eine energiebezogene Größe aus den tatsächlichen Voice-Envelopes ergänzt bzw. ersetzt. Die Kompensation orientiert sich an der Quadrat-Summe der Envelope-Gewichte und wird mit einer festen, konservativen Voice-Skalierung sowie einem definierten Ceiling versehen. Dadurch erhält die unkorrelierte Summe mehr RMS-Energie als bei reiner Summen-Normalisierung, während korrelierte Quellen nicht unkontrolliert anwachsen.

Die Kompensation bleibt geglättet und wird nicht sampleweise aus einer Division durch die momentane Envelope-Summe abgeleitet. Ein minimaler Gain-Floor verhindert, dass kurze Envelope-Täler die gesamte Textur hörbar absenken; ein maximaler Gain-Faktor und die bestehenden endlichen Output-Prüfungen schützen vor Runaway-Amplification. Die genaue konservative Skalierung wird anhand der RMS-Tests abgestimmt. Eine reine feste Verstärkung wird verworfen, weil sie unabhängig von der Zahl aktiver Voices entweder zu leise oder bei korreliertem Material zu laut wäre.

### Bounded pitch random walk

Beim Erzeugen eines neuen Drift-Ziels wird nicht mehr für jede Voice ein unabhängiger absoluter Speed-Wert aus dem gesamten Bereich gewählt. Stattdessen wird vom bisherigen Ziel ein zufälliger, durch Pitch-Amount und eine interne Schrittweite begrenzter Delta-Schritt addiert und anschließend auf den globalen Playback-Speed-Bereich geklemmt. So kann Drift 1.0 den vollständigen Bereich über mehrere Zielwechsel erreichen, ohne bei jedem Wechsel eine maximale Gegenbewegung zu verlangen. Bei niedrigem Drift bleiben Schritte und Voice-Differenzen klein.

Die Zielwechsel bleiben nicht-periodisch und seed-deterministisch. Die Playback-Speed-Annäherung erhält eine längere Zeitkonstante als die Positions-/Stereo-Bewegung, nominal etwa 800 ms. Damit werden sowohl Random-Walk-Schritte als auch Änderungen des sichtbaren Drift-Werts kontinuierlich verfolgt. Drift 0 setzt neue Ziele auf `1.0`; die laufende Rückkehr dorthin bleibt geglättet.

### Continuous safety changes

Beim Ändern von Drift werden neue Safe-Read-Grenzen sofort berechnet, aber laufende hörbare Offsets nicht durch eine direkte Positionsverschiebung ersetzt. Offset-Ziele werden an die neuen Grenzen geklemmt und der aktuelle Offset wird mit der normalen Bewegungs-Glättung dorthin geführt. Falls der Read-Kopf selbst eine Grenze erreicht, geschieht der Voice-Neustart weiterhin nur am Envelope-Ende bzw. bei Near-Zero-Gain. Die konservative maximale Speed-Reserve bleibt bestehen, damit die langsamere Zielbewegung keine ungültige Leseposition erzeugt.

### Verification and tuning

Tests verwenden deterministische Quellen und Seeds. Für Lautheit werden stabile Freeze-Fenster nach dem Aktivierungsübergang mit einer kontrollierten Referenz verglichen; für Transienten werden maximale benachbarte Output-Schritte sowie die Änderung zwischen aufeinanderfolgenden beobachtbaren Renderfenstern geprüft. Die bestehenden Boundary-, Determinismus- und Finite-Output-Tests bleiben unverändert und müssen weiterhin bestehen. Tuning-Parameter bleiben interne Konstanten, damit die UI unverändert bleibt.

## Risks / Trade-offs

- [Korrelierte Voices werden durch energiebezogene Mischung zu laut] → konservatives Voice-Gain-Ceiling, RMS-Tests mit korreliertem und decorreliertem Material sowie finite Output-Grenzen.
- [Die Textur bleibt wegen zu niedrigem Gain leise] → RMS-Vergleich gegen die Captured-Source-Referenz und expliziter Mindestpegel für typische Voice-Überlappung.
- [Random-Walk erreicht bei hoher Drift den vollen Bereich zu langsam] → Schrittweite und Zielwechsel-Intervall getrennt abstimmen; hohe Drift darf größere Schritte verwenden, ohne absolute Vollbereich-Sprünge zu erlauben.
- [Positions-Offset bleibt bei einer Safety-Grenze kurz außerhalb des idealen Bereichs] → Zielwerte sofort klemmen, aktuellen hörbaren Offset glätten und Read-/Restart-Logik weiterhin strikt auf sichere Bereiche begrenzen.
- [Sehr impulsives Material macht Speed-Glides weiterhin hörbar] → zusätzliche Kontinuitätsmessungen beibehalten; das Verhalten wird geglättet, aber nicht in einen spektralen Pitch-Shifter umgewandelt.

## Migration Plan

Keine Parameter- oder Preset-Migration erforderlich. Die Änderung wird als interne DSP-Anpassung mit neuen Regressionstests ausgerollt. Bei unpassender klanglicher Abstimmung können Gain-Ceiling, Random-Walk-Schrittweite und Smoothing-Konstante zurückgesetzt werden, ohne gespeicherte Plugin-Zustände zu ändern.
