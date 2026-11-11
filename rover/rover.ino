/*
  Mars-Rover – Hauptprogramm
  ==========================
  Liest alle Sensoren aus und zeigt die Werte im seriellen Monitor (9600 Baud)
  und auf dem LCD-Display an.
  Der Rover reagiert noch nicht auf die Werte – das kommt später hier in loop().

  Jeder Teil hat seinen eigenen Tab:
    neigung.ino       – Neigungssensor KY-027          (Pin D2, LED an D9)
    helligkeit.ino    – Helligkeitssensor              (Pin A3)
    feuchtigkeit.ino  – Temperatur/Feuchtigkeit DHT11  (Pin D4)
    anzeige.ino       – LCD-Display 16x2 mit I2C       (SDA an A4, SCL an A5)

  Benötigte Bibliotheken (Werkzeuge -> Bibliotheken verwalten):
    - Bounce2 (von Thomas O Fredericks)
    - DHT sensor library (von Adafruit), dazu "Adafruit Unified Sensor" mitinstallieren
    - hd44780 (von Bill Perry)

  Wichtig: Kein delay() in loop()! Sonst reagiert der Neigungssensor zu spät.
  Wann welcher Sensor ausgelesen wird, wird unten mit millis() geregelt.
*/

const unsigned long HELLIGKEIT_INTERVALL_MS = 500;     // alle 0,5 Sekunden
const unsigned long FEUCHTIGKEIT_INTERVALL_MS = 2000;  // alle 2 Sekunden (öfter kann der DHT11 nicht)

unsigned long letzteHelligkeitMessung = 0;
unsigned long letzteFeuchtigkeitMessung = 0;

// Die letzten Messwerte, damit das Display sie jederzeit anzeigen kann
int aktuelleHelligkeit = 0;
float aktuelleTemperatur = NAN;    // NAN = noch kein Wert gemessen
float aktuelleFeuchtigkeit = NAN;

void setup() {
  Serial.begin(9600);
  delay(500);  // Sensoren nach dem Einschalten kurz zur Ruhe kommen lassen
  neigungSetup();
  feuchtigkeitSetup();
  anzeigeSetup();
  Serial.println("Rover bereit");
}

void loop() {
  unsigned long jetzt = millis();

  // Neigung: wird bei JEDEM Durchlauf geprüft, damit keine Änderung verpasst wird
  int neigung = neigungUpdate();
  if (neigung == 1)  Serial.println("Neigung steigt");
  if (neigung == -1) Serial.println("Neigung sinkt");

  // Helligkeit: alle 0,5 Sekunden
  if (jetzt - letzteHelligkeitMessung >= HELLIGKEIT_INTERVALL_MS) {
    letzteHelligkeitMessung = jetzt;
    aktuelleHelligkeit = helligkeitMessen();
    Serial.print("Helligkeit: ");
    Serial.println(aktuelleHelligkeit);
  }

  // Temperatur und Feuchtigkeit: alle 2 Sekunden
  if (jetzt - letzteFeuchtigkeitMessung >= FEUCHTIGKEIT_INTERVALL_MS) {
    letzteFeuchtigkeitMessung = jetzt;
    aktuelleTemperatur = temperaturMessen();
    aktuelleFeuchtigkeit = feuchtigkeitMessen();

    if (isnan(aktuelleTemperatur) || isnan(aktuelleFeuchtigkeit)) {
      Serial.println("Feuchtigkeitssensor: Fehler beim Lesen (Verkabelung pruefen)");
    } else {
      Serial.print("Temperatur: ");
      Serial.print(aktuelleTemperatur);
      Serial.println(" °C");

      Serial.print("Feuchtigkeit: ");
      Serial.print(aktuelleFeuchtigkeit);
      Serial.println(" %");
    }
  }

  // Display: zeigt immer die neuesten Werte (macht nichts, wenn kein LCD angeschlossen ist)
  anzeigeUpdate(aktuelleFeuchtigkeit, aktuelleTemperatur, aktuelleHelligkeit, neigungGeneigt());
}
