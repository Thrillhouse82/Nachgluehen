## Why

Die aktuelle Freeze-Wiedergabe behandelt den kurzen Capture-Buffer im Kern als einzelnen Loop. Dadurch werden Transienten und der Loop-Anfang regelmäßig wiederholt, sodass ein geloopter Ton oder Akkord statt einer kontinuierlichen, schwebenden Fläche entsteht.

Freeze soll das eingefrorene Signal künftig als Material-Pool für eine überlappende Texture-Wiedergabe verwenden. Damit bleibt das Quellmaterial erkennbar, während feste Wiederholungsanfänge und hörbare Loop-Perioden reduziert werden.

## What Changes

- Die interne `LivingFreezeEngine`-Wiedergabe wird von einer globalen linearen Playback-Position auf mehrere feste, unabhängige Read-Heads bzw. Wiedergabefenster erweitert.
- Wiedergabefenster starten an unterschiedlichen Positionen im Capture, überlappen sich und verwenden weiche kontinuierliche Envelopes.
- Fenster erhalten eigene Read-Positionen und Phasen; bei positivem Drift können zusätzlich Geschwindigkeit, Position, Länge, Stereo-Abweichung und zeitliche Einsätze geglättet und deterministisch variieren.
- `Drift = 0 %` verwendet weiterhin die vollständige überlappende Texture-Architektur, jedoch ohne zusätzliche pseudozufällige Modulation.
- Freeze-Aktivierung und -Deaktivierung sowie der Start und das Ende einzelner Fenster bleiben weich und klickfrei.
- Capture-Dauer, sichtbare Parameter (`Freeze`, `Drift`, `Dry/Wet`) und Echtzeitsicherheitsanforderungen bleiben unverändert.
- JUCE-Unit-Tests decken kontinuierliche Ausgabe, weiche Fenstergrenzen, fehlende klassische Buffer-Wiederholung, Drift-Determinismus und endliche Ausgabewerte ab.

## Capabilities

### New Capabilities

<!-- Keine neue Nutzerfunktionalität; die bestehende Freeze-Capability wird erweitert. -->

### Modified Capabilities

- `living-freeze`: Die Anforderungen an Freeze-Wiedergabe, interne Grenzen, Drift bei null und positivem Drift sowie die zugehörigen Testfälle werden auf eine überlappende Texture-Engine konkretisiert.

## Impact

- Betroffen sind primär `LivingFreezeEngine` und die JUCE-Unit-Tests.
- Die Engine benötigt feste, vorab reservierte Zustände für mehrere Stimmen; Heap-Allokationen im Audio-Callback bleiben ausgeschlossen.
- Öffentliche Plugin-Parameter und UI bleiben unverändert. Bestehende Capture-, Dry/Wet- und Freeze-Übergangslogik wird weiterverwendet oder intern angepasst.
