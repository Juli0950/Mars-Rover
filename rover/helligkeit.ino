// Helligkeitssensor (analog) – braucht kein Setup

const int HELLIGKEIT_PIN = A3;

// Rückgabe: Helligkeit als Stufe von 1 bis 10
int helligkeitMessen() {
  int helligkeit = analogRead(HELLIGKEIT_PIN);

  // Helligkeitswert von 0-1023 auf 1-10 umwandeln.
  // (0, 1024, 1, 11) statt (0, 1023, 1, 10): Sonst gibt es die 10 nur bei genau 1023,
  // so kommt jede Stufe gleich oft vor.
  return map(helligkeit, 0, 1024, 1, 11);
}
