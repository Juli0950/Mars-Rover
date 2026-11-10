/*
  KY-027 Neigungssensor für den Mars-Rover
  Meldet im seriellen Monitor (9600 Baud), ob die Neigung steigt oder sinkt.

  Benötigt die Bibliothek "Bounce2" von Thomas O Fredericks
  (Arduino IDE: Werkzeuge -> Bibliotheken verwalten -> "Bounce2" suchen -> Installieren)

  Verkabelung: G -> GND, + -> 5V, S -> D2, L -> D9 (LED, optional)

  Der Sensor kennt nur "gerade" oder "geneigt". Beim Start muss er GERADE stehen,
  diese Lage merkt sich der Code als "gerade".
*/

#include <Bounce2.h>

const int SENSOR_PIN = 2;
const int LED_PIN = 9;

Bounce sensor;    // die Bibliothek übernimmt das Entprellen
int geradePegel;  // Pegel am Sensor-Pin, wenn der Sensor gerade steht

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  delay(500);                               // Sensor nach dem Einschalten zur Ruhe kommen lassen
  sensor.attach(SENSOR_PIN, INPUT_PULLUP);
  sensor.interval(50);                      // Signal muss 50 ms stabil sein
  geradePegel = sensor.read();              // Startlage = "gerade"
  Serial.println("Neigungssensor bereit");
}

void loop() {
  sensor.update();
  if (sensor.changed()) {
    bool geneigt = sensor.read() != geradePegel;
    Serial.println(geneigt ? "Neigung steigt" : "Neigung sinkt");
    digitalWrite(LED_PIN, geneigt);         // LED an, solange geneigt
  }
}
