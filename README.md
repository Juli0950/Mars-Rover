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
| LCD-Display 20x4 mit I2C (SDA) | A4 |
| LCD-Display 20x4 mit I2C (SCL) | A5 |

Neue Sensoren bitte hier eintragen, damit kein Pin doppelt belegt wird.

## LCD-Display

Sunstar 2004A (20 Zeichen × 4 Zeilen) mit I2C-Modul. Zeigt Feuchtigkeit, Temperatur,
Helligkeit und Neigung live an, jeden Wert in einer eigenen Zeile. In `rover/anzeige.ino`
kann man oben umschalten:

- `BALKENANZEIGE = true`: Werte mit Balken, die live mitwachsen, dazu ein Neigungs-Symbol
  (flache Linie = gerade, Rampe = schief)
- `BALKENANZEIGE = false`: nur die Werte als Text

Ist beim Start kein Display angeschlossen, läuft der Rover normal weiter.
Zeigt das Display nur Kästchen oder gar nichts, am blauen Drehregler hinten den Kontrast einstellen.

## Benötigte Bibliotheken

In der Arduino IDE unter *Werkzeuge → Bibliotheken verwalten* installieren:

- **Bounce2** (von Thomas O Fredericks)
- **DHT sensor library** (von Adafruit) – dabei „Adafruit Unified Sensor“ mitinstallieren
- **hd44780** (von Bill Perry) – für das LCD-Display

## Regeln für neue Sensoren

1. Eigener Tab im Ordner `rover/`, mit Funktionen wie `xxxSetup()` und `xxxMessen()` – kein eigenes `setup()`/`loop()`.
2. Kein `delay()` in `loop()` – Zeitabstände mit `millis()` regeln.
3. Namen mit Präfix (`helligkeitPin` statt `sensor`), damit nichts doppelt heißt.
4. `Serial.begin()` nur im Hauptprogramm.
