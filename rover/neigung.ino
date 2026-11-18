// Neigung mit zwei Neigungssensoren KY-027 – benötigt die Bibliothek "Bounce2"
//
// Jeder Sensor erkennt nur EINE Richtung, weil er schräg eingebaut ist (siehe README):
//   Sensor "bergauf" löst aus, wenn die Front des Rovers nach oben zeigt
//   Sensor "bergab"  löst aus, wenn die Front des Rovers nach unten zeigt
// Zusammen ergibt das: Neigung steigt, sinkt oder der Rover steht gerade.
//
// Beim Start muss der Rover auf GERADEM Boden stehen: Diese Lage merkt sich der Code
// als "gerade". Nach jeder Änderung am Einbau also gerade hinstellen und Reset drücken.
#include <Bounce2.h>

const int BERGAUF_PIN = 2;       // S-Pin vom Sensor "bergauf"
const int BERGAUF_LED_PIN = 9;   // L-Pin vom Sensor "bergauf" (LED, optional)
const int BERGAB_PIN = 3;        // S-Pin vom Sensor "bergab"
const int BERGAB_LED_PIN = 10;   // L-Pin vom Sensor "bergab" (LED, optional)

// So lange muss ein Sensor stabil sein, bevor es zählt.
// Länger als bei einem einzelnen Sensor, damit Ruckeln beim Fahren nicht als Neigung zählt.
const unsigned long NEIGUNG_ENTPRELL_MS = 200;

Bounce bergaufSensor;
Bounce bergabSensor;
int bergaufGeradePegel;   // Pegel der Sensoren, wenn der Rover gerade steht
int bergabGeradePegel;
int neigungZustand = GERADE;

void neigungSetup() {
  pinMode(BERGAUF_LED_PIN, OUTPUT);
  pinMode(BERGAB_LED_PIN, OUTPUT);
  bergaufSensor.attach(BERGAUF_PIN, INPUT_PULLUP);
  bergabSensor.attach(BERGAB_PIN, INPUT_PULLUP);
  bergaufSensor.interval(NEIGUNG_ENTPRELL_MS);
  bergabSensor.interval(NEIGUNG_ENTPRELL_MS);
  bergaufGeradePegel = bergaufSensor.read();  // Startlage = "gerade"
  bergabGeradePegel = bergabSensor.read();
}

// Liest beide Sensoren. Rückgabe: true, wenn sich die Neigung gerade geändert hat
bool neigungUpdate() {
  bergaufSensor.update();
  bergabSensor.update();

  bool bergauf = bergaufSensor.read() != bergaufGeradePegel;
  bool bergab = bergabSensor.read() != bergabGeradePegel;

  // Die LED auf jedem Sensor zeigt, ob er ausgelöst hat (hilft beim Einbauen)
  digitalWrite(BERGAUF_LED_PIN, bergauf);
  digitalWrite(BERGAB_LED_PIN, bergab);

  int neu;
  if (bergauf && bergab) {
    neu = UNKLAR;  // beide gleichzeitig geht eigentlich nicht, z. B. Rover hochgehoben
  } else if (bergauf) {
    neu = STEIGT;
  } else if (bergab) {
    neu = SINKT;
  } else {
    neu = GERADE;
  }

  if (neu == neigungZustand) return false;
  neigungZustand = neu;
  return true;
}

// Rückgabe: GERADE, STEIGT, SINKT oder UNKLAR
int neigungLesen() {
  return neigungZustand;
}

// Rückgabe: der Zustand als Wort, z. B. "steigt"
const char* neigungText(int zustand) {
  if (zustand == STEIGT) return "steigt";
  if (zustand == SINKT) return "sinkt";
  if (zustand == UNKLAR) return "unklar";
  return "gerade";
}
