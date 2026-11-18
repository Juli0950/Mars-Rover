/*
  Mars-Rover – Hauptprogramm
  ==========================
  Liest alle Sensoren aus und zeigt die Werte im seriellen Monitor (9600 Baud)
  und auf dem LCD-Display an.
  Der Rover reagiert noch nicht auf die Werte – das kommt später hier in loop().

  Jeder Teil hat seinen eigenen Tab:
    neigung.ino       – 2 Neigungssensoren KY-027      (bergauf: D2 + LED D9, bergab: D3 + LED D10)
    helligkeit.ino    – Helligkeitssensor in Lux       (Pin A3)
    feuchtigkeit.ino  – Temperatur/Feuchtigkeit DHT11  (Pin D4)
    anzeige.ino       – LCD-Display 20x4 mit I2C       (SDA an A4, SCL an A5)

  Benötigte Bibliotheken (Werkzeuge -> Bibliotheken verwalten):
    - Bounce2 (von Thomas O Fredericks)
    - DHT sensor library (von Adafruit), dazu "Adafruit Unified Sensor" mitinstallieren
    - hd44780 (von Bill Perry)

  Wichtig: Kein delay() in loop()! Sonst reagieren die Neigungssensoren zu spät.
  Wann welcher Sensor ausgelesen wird, wird unten mit millis() geregelt.
*/

// Neigungs-Zustände. Sie stehen hier im Haupt-Tab, weil mehrere Tabs sie benutzen.
const int GERADE = 0;
const int STEIGT = 1;    // Front zeigt nach oben (bergauf)
const int SINKT = -1;    // Front zeigt nach unten (bergab)
const int UNKLAR = 2;    // beide Sensoren ausgelöst, z. B. Rover hochgehoben

const unsigned long HELLIGKEIT_INTERVALL_MS = 500;     // alle 0,5 Sekunden
const unsigned long FEUCHTIGKEIT_INTERVALL_MS = 2000;  // alle 2 Sekunden (öfter kann der DHT11 nicht)

unsigned long letzteHelligkeitMessung = 0;
unsigned long letzteFeuchtigkeitMessung = 0;

// Die letzten Messwerte, damit das Display sie jederzeit anzeigen kann
float aktuelleLux = 0;
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
  if (neigungUpdate()) {
    Serial.print("Neigung: ");
    Serial.println(neigungText(neigungLesen()));
  }

  // Helligkeit: alle 0,5 Sekunden
  if (jetzt - letzteHelligkeitMessung >= HELLIGKEIT_INTERVALL_MS) {
    letzteHelligkeitMessung = jetzt;
    aktuelleLux = helligkeitMessen();
    Serial.print("Helligkeit: ");
    Serial.print(aktuelleLux, 0);  // 0 Nachkommastellen
    Serial.println(" lx");
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
  anzeigeUpdate(aktuelleFeuchtigkeit, aktuelleTemperatur, aktuelleLux, neigungLesen());
}
