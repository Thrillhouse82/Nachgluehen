## 1. Output-Gain-Parameter und DSP-Staging

- [x] 1.1 Einen host-automatisierbaren Output-Gain-Parameter mit Mute-Endpunkt, +12-dB-Maximum und 0-dB-Standardwert ergänzen und die State-Wiederherstellung absichern.
- [x] 1.2 Die geglättete finale Gain-Stufe nach dem LivingFreezeEngine-Dry/Wet-Mix implementieren, einschließlich finiter Mute- und Boost-Verarbeitung ohne Limiter.
- [x] 1.3 Eine echtzeitsichere, lock-freie Clip-Hold-Erkennung für den finalen Post-Gain-Ausgang bei -0.1 dBFS ergänzen.

## 2. Editor-Erweiterung

- [x] 2.1 Einen an den Output-Gain-Parameter gebundenen Output-Drehregler samt dB-Anzeige in das feste Editor-Layout integrieren.
- [x] 2.2 Eine kleine rote `CLIP`-Anzeige neben dem Output-Regler implementieren, die den mindestens einsekündigen Hold-Status darstellt.

## 3. Tests und Validierung

- [x] 3.1 Parameter-, Host-Automations- und State-Tests für Output Gain bei Mute, 0 dB und +12 dB ergänzen.
- [x] 3.2 Tests für Post-Mix-Gain-Staging, Gain-Kontinuität, finite Werte und unverändertes Verhalten bei 0 dB ergänzen.
- [x] 3.3 Tests für Clip-Schwelle, mindestens einsekündigen Hold und Rückkehr der Anzeige in den inaktiven Zustand ergänzen.
- [x] 3.4 Projekt bauen, die vollständige JUCE-Test-Suite ausführen und den OpenSpec-Change strikt validieren.
