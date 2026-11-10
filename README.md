# Mars-Rover

NWT-Projekt: Arduino-Code für unseren Mars-Rover.

## Ordner

| Ordner | Inhalt |
|---|---|
| `rover/` | **Hauptprogramm** – alle Sensoren in einem Sketch (ein Tab pro Sensor) |
| `neigungssensor/` | Neigungssensor einzeln zum Testen |

## Pin-Plan (Arduino Uno)

| Sensor | Pin |
|---|---|
| Neigungssensor KY-027 (Signal S) | D2 |
| Neigungssensor KY-027 (LED L) | D9 |
| Temperatur/Feuchtigkeit DHT11 | D4 |
| Helligkeitssensor | A3 |

Neue Sensoren bitte hier eintragen, damit kein Pin doppelt belegt wird.

## Benötigte Bibliotheken

In der Arduino IDE unter *Werkzeuge → Bibliotheken verwalten* installieren:

- **Bounce2** (von Thomas O Fredericks)
- **DHT sensor library** (von Adafruit) – dabei „Adafruit Unified Sensor“ mitinstallieren

## Regeln für neue Sensoren

1. Eigener Tab im Ordner `rover/`, mit Funktionen wie `xxxSetup()` und `xxxMessen()` – kein eigenes `setup()`/`loop()`.
2. Kein `delay()` in `loop()` – Zeitabstände mit `millis()` im Hauptprogramm regeln.
3. Namen mit Präfix (`helligkeitPin` statt `sensor`), damit nichts doppelt heißt.
4. `Serial.begin()` nur im Hauptprogramm.
