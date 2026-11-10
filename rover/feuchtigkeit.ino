// Temperatur- und Feuchtigkeitssensor DHT11 – benötigt die "DHT sensor library" von Adafruit
#include <DHT.h>

const int FEUCHTIGKEIT_PIN = 4;  // vorher Pin 2, der ist jetzt vom Neigungssensor belegt

DHT dhtSensor(FEUCHTIGKEIT_PIN, DHT11);

void feuchtigkeitSetup() {
  dhtSensor.begin();
}

// Rückgabe: Temperatur in °C, oder NAN bei einem Lesefehler
float temperaturMessen() {
  return dhtSensor.readTemperature();
}

// Rückgabe: Luftfeuchtigkeit in %, oder NAN bei einem Lesefehler
float feuchtigkeitMessen() {
  return dhtSensor.readHumidity();
}
