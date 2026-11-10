// Neigungssensor KY-027 – benötigt die Bibliothek "Bounce2"
// Der Sensor kennt nur "gerade" oder "geneigt". Beim Start muss er GERADE stehen,
// diese Lage merkt sich der Code als "gerade".
#include <Bounce2.h>

const int NEIGUNG_PIN = 2;
const int NEIGUNG_LED_PIN = 9;

Bounce neigungSensor;    // die Bibliothek übernimmt das Entprellen
int neigungGeradePegel;  // Pegel am Sensor-Pin, wenn der Sensor gerade steht

void neigungSetup() {
  pinMode(NEIGUNG_LED_PIN, OUTPUT);
  neigungSensor.attach(NEIGUNG_PIN, INPUT_PULLUP);
  neigungSensor.interval(50);                 // Signal muss 50 ms stabil sein
  neigungGeradePegel = neigungSensor.read();  // Startlage = "gerade"
}

// Rückgabe: 1 = Neigung steigt, -1 = Neigung sinkt, 0 = keine Änderung
int neigungUpdate() {
  neigungSensor.update();
  if (!neigungSensor.changed()) return 0;

  bool geneigt = neigungSensor.read() != neigungGeradePegel;
  digitalWrite(NEIGUNG_LED_PIN, geneigt);     // LED an, solange geneigt
  return geneigt ? 1 : -1;
}
