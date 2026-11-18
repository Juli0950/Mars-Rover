// Helligkeitssensor mit Fotowiderstand (z. B. KY-018) – gibt die Helligkeit in Lux zurück
//
// So funktioniert es: Der Fotowiderstand (LDR) bildet mit einem festen Widerstand einen
// Spannungsteiler. Aus dem Messwert an A3 rechnet der Code zuerst den Widerstand des LDR aus
// und daraus mit der Kennlinie des LDR die Helligkeit in Lux.
//
// Achtung: Ein Fotowiderstand ist kein genaues Messgerät. Die Lux-Werte sind nur ungefähr
// richtig. Mit LUX_KORREKTUR könnt ihr sie an ein Lux-Messgerät oder eine Handy-App anpassen.

const int HELLIGKEIT_PIN = A3;

// ---------- Einstellungen (an euren Sensor anpassen) ----------
const float VORWIDERSTAND = 10000;  // fester Widerstand im Spannungsteiler in Ohm (KY-018: 10 kΩ)
const float LDR_R10 = 15000;        // Widerstand des LDR bei 10 Lux in Ohm (GL5528: 10 bis 20 kΩ)
const float LDR_GAMMA = 0.6;        // Steilheit der LDR-Kennlinie (GL5528: ca. 0,6)
const float LUX_KORREKTUR = 1.0;    // Zeigt der Rover 200 lx und das Messgerät 300 lx -> 1.5

// Test: Hand über den Sensor halten -> der Lux-Wert muss KLEINER werden.
// Wird er stattdessen größer, hier true durch false ersetzen.
const bool LDR_AN_GND = true;  // true = LDR zwischen Signal und GND, false = zwischen 5V und Signal

const float LUX_MAX = 99999;  // mehr passt nicht aufs Display (pralle Sonne hat ca. 100000 lx)

// Rückgabe: Helligkeit in Lux
float helligkeitMessen() {
  int roh = analogRead(HELLIGKEIT_PIN);
  roh = constrain(roh, 1, 1022);  // bei 0 oder 1023 würde unten durch 0 geteilt

  // 1. Widerstand des LDR aus dem Spannungsteiler ausrechnen
  float ldr;
  if (LDR_AN_GND) {
    ldr = VORWIDERSTAND * roh / (1023.0 - roh);
  } else {
    ldr = VORWIDERSTAND * (1023.0 - roh) / roh;
  }

  // 2. Kennlinie des LDR: Bei 10 Lux hat er LDR_R10 Ohm, bei mehr Licht weniger
  float lux = 10 * pow(LDR_R10 / ldr, 1 / LDR_GAMMA) * LUX_KORREKTUR;
  if (lux > LUX_MAX) lux = LUX_MAX;
  return lux;
}
