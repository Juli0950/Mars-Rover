/*
  KY-027 Magic Light Cup – Neigungssensor für den Mars-Rover
  ==========================================================

  Was macht der Code?
  -------------------
  Er meldet im seriellen Monitor (9600 Baud), ob die Neigung steigt oder sinkt,
  und schaltet die LED auf dem Modul an, solange der Sensor geneigt ist.

  Wichtig zum Sensor:
  -------------------
  Im KY-027 steckt ein Neigungsschalter (Quecksilber- bzw. Kugelschalter).
  Der kennt nur ZWEI Zustände: "gerade" oder "geneigt" – einen genauen Winkel
  kann er nicht messen. Deshalb bedeutet hier:
    - "Neigung steigt" = Sensor kippt von gerade  -> geneigt
    - "Neigung sinkt"  = Sensor kippt von geneigt -> gerade

  Verkabelung (KY-027 -> Arduino Uno/Nano):
  -----------------------------------------
    G  -> GND
    +  -> 5V
    S  -> D2   (Signal des Neigungsschalters)
    L  -> D9   (LED auf dem Modul, optional)

  Beim Einschalten bzw. nach dem Reset muss der Sensor GERADE stehen:
  Die Lage beim Start wird als "gerade" gespeichert. Dadurch ist egal,
  ob euer Modul im geraden Zustand HIGH oder LOW liefert.
*/

// ---------- Einstellungen ----------
const int SENSOR_PIN = 2;                   // S-Pin des KY-027
const int LED_PIN = 9;                      // L-Pin des KY-027
const unsigned long ENTPRELL_ZEIT_MS = 50;  // so lange muss das Signal stabil sein

// ---------- Zustand ----------
int geradePegel;                  // Pegel am Sensor-Pin, wenn der Sensor gerade steht
bool geneigt = false;             // aktueller (entprellter) Zustand
int letzterRohwert;               // letzter direkt gelesener Wert
unsigned long letzteAenderung = 0;

// Liest den Sensor (entprellt) und meldet, ob sich die Neigung geändert hat.
// Rückgabe:  1 = Neigung steigt
//           -1 = Neigung sinkt
//            0 = keine Änderung
// Die Funktion blockiert nicht (kein delay) und kann später direkt
// in den Rover-Code übernommen werden.
int pruefeNeigung() {
  int rohwert = digitalRead(SENSOR_PIN);

  // Der Schalter "prellt" beim Kippen (springt kurz hin und her).
  // Erst wenn sich der Wert ENTPRELL_ZEIT_MS lang nicht ändert, zählt er.
  if (rohwert != letzterRohwert) {
    letzterRohwert = rohwert;
    letzteAenderung = millis();
    return 0;
  }
  if (millis() - letzteAenderung < ENTPRELL_ZEIT_MS) {
    return 0;
  }

  bool jetztGeneigt = (rohwert != geradePegel);
  if (jetztGeneigt == geneigt) {
    return 0;  // nichts hat sich geändert
  }

  geneigt = jetztGeneigt;
  return geneigt ? 1 : -1;
}

void setup() {
  Serial.begin(9600);
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  delay(500);  // Sensor nach dem Einschalten kurz zur Ruhe kommen lassen
  geradePegel = digitalRead(SENSOR_PIN);
  letzterRohwert = geradePegel;

  Serial.println(F("KY-027 Neigungssensor bereit."));
  Serial.println(F("Startlage wurde als 'gerade' gespeichert."));
}

void loop() {
  int trend = pruefeNeigung();

  if (trend == 1) {
    Serial.println(F("Neigung steigt  -> Rover wird schraeg"));
  } else if (trend == -1) {
    Serial.println(F("Neigung sinkt   -> Rover wird wieder gerade"));
  }

  // LED leuchtet, solange der Sensor geneigt ist
  digitalWrite(LED_PIN, geneigt ? HIGH : LOW);
}
