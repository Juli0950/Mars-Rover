# Mars-Rover

NWT-Projekt: Arduino-Code für unseren Mars-Rover.

## Ordner

| Ordner | Inhalt |
|---|---|
| `rover/` | **Hauptprogramm** – alle Sensoren in einem Sketch (ein Tab pro Sensor) |
| `neigungssensor/` | ein einzelner Neigungssensor zum Testen |

## Pin-Plan (Arduino Uno)

| Bauteil | Pin am Bauteil | Arduino |
|---|---|---|
| Neigungssensor A „bergauf“ (KY-027) | S | D2 |
| | L (LED) | D9 |
| Neigungssensor B „bergab“ (KY-027) | S | D3 |
| | L (LED) | D10 |
| Temperatur/Feuchtigkeit DHT11 | S / DATA | D4 |
| Helligkeitssensor | S / AO | A3 |
| LCD-Display 20x4 mit I2C | SDA | A4 |
| | SCL | A5 |

Plus (`+`, `VCC`) und Minus (`-`, `G`, `GND`) aller Bauteile kommen über die Leisten
am Steckbrett an 5V und GND. Neue Sensoren bitte hier eintragen, damit kein Pin doppelt belegt wird.

## Neigung: zwei Sensoren für bergauf und bergab

Ein einzelner KY-027 merkt nur „gerade“ oder „gekippt“, aber nicht, in welche Richtung.
Deshalb gibt es zwei Sensoren, die in entgegengesetzte Richtungen zeigen:

| Sensor A „bergauf“ (D2) | Sensor B „bergab“ (D3) | Anzeige |
|---|---|---|
| aus | aus | gerade |
| **an** | aus | steigt |
| aus | **an** | sinkt |
| **an** | **an** | unklar (z. B. Rover hochgehoben) |

### So funktioniert der Sensor

Der Schalter auf dem KY-027 ist das kleine Röhrchen, das auf der Platine steht.
Darin ist eine Kugel (oder ein Tropfen Quecksilber), die immer zur tiefsten Stelle rollt.
Der Schalter ist nur geschlossen, wenn sie unten an der Platine liegt, denn dort sind die Kontakte.

### Einbau

```
Seitenansicht, der Rover fährt nach rechts →

          Sensor B (bergab)        Sensor A (bergauf)
                ==|                      |==
   +--------------------------------------------------+
   |                      Rover                       |  vorne
   +--------------------------------------------------+
       (O)                                      (O)

   |   = Platine, steht senkrecht und quer zur Fahrtrichtung
   ==  = Röhrchen, liegt waagerecht in Fahrtrichtung
```

- **Sensor A (bergauf):** Platine steht senkrecht, das Röhrchen zeigt **nach vorne**.
  Hebt sich die Front, wird das Platinen-Ende tiefer, die Kugel rollt zu den Kontakten und der Sensor löst aus.
- **Sensor B (bergab):** genauso, aber das Röhrchen zeigt **nach hinten**.
- Die **Spitze der Röhrchen leicht nach unten** neigen (ca. 5–10°). Sonst rollt die Kugel
  schon auf geradem Boden hin und her. Je weiter nach unten, desto steiler muss der Hang sein.
- Beide Sensoren **genau in Fahrtrichtung** ausrichten, sonst lösen sie auch aus, wenn der Rover zur Seite kippt.
- **Mittig auf dem Chassis** befestigen (dort ruckelt es am wenigsten), nicht auf Rädern, Achsen oder Servos.
- **Fest anbringen**, z. B. mit Heißkleber oder Schrauben an einem kleinen L-Winkel. Wackelt der Sensor,
  verstellt sich der Winkel.

### Einstellen mit den LEDs

Die LED auf jedem Sensor leuchtet, sobald er auslöst.

1. Rover auf einen geraden Tisch stellen und **Reset** drücken. Beide LEDs sind aus.
2. **Front anheben** (z. B. Bücher unter die Vorderräder). Ab dem gewünschten Winkel muss die LED von Sensor A angehen.
   - Geht sie zu früh an: Röhrchen-Spitze weiter nach unten neigen.
   - Geht sie zu spät oder gar nicht an: Spitze weniger nach unten neigen.
3. Gerade hinstellen, dann **Heck anheben**. Jetzt muss die LED von Sensor B angehen.
4. Geht eine LED beim falschen Anheben an, ist der Sensor falsch herum eingebaut.
5. Nach jeder Änderung am Einbau: Rover gerade hinstellen und Reset drücken.

Wichtig: Beim Einschalten muss der Rover immer auf geradem Boden stehen. Diese Lage merkt sich der Code als „gerade“.

## Helligkeit in Lux

Der Code rechnet den Messwert des Fotowiderstands in Lux um. Ein Fotowiderstand ist aber kein
Messgerät, die Werte sind nur ungefähr richtig. Einstellungen oben in `rover/helligkeit.ino`:

- **Richtung prüfen:** Hand über den Sensor halten, dann muss der Lux-Wert kleiner werden.
  Wird er größer, `LDR_AN_GND` von `true` auf `false` ändern.
- `VORWIDERSTAND`: der feste Widerstand neben dem Fotowiderstand (beim KY-018 10 kΩ, Aufdruck „103“).
- `LUX_KORREKTUR`: Mit einem Lux-Messgerät oder einer Lux-Messer-App auf dem Handy vergleichen.
  Zeigt der Rover 200 lx und das Messgerät 300 lx, dann `1.5` eintragen.

Für genaue Lux-Werte gibt es den Sensor **BH1750**. Er misst direkt in Lux und kommt einfach mit an SDA/SCL (A4/A5), wie das Display.

## LCD-Display

Sunstar 2004A (20 Zeichen × 4 Zeilen) mit I2C-Modul. Zeigt Feuchtigkeit, Temperatur,
Helligkeit und Neigung live an, jeden Wert in einer eigenen Zeile. In `rover/anzeige.ino`
kann man oben umschalten:

- `BALKENANZEIGE = true`: Werte mit Balken, die live mitwachsen, dazu ein Neigungs-Symbol
  (flache Linie = gerade, Rampe nach oben = steigt, Rampe nach unten = sinkt)
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
