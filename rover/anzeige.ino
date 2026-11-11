/*
  LCD-Display 16x2 mit I2C-Modul – zeigt alle Messwerte live an
  =============================================================
  Benötigt die Bibliothek "hd44780" von Bill Perry. Die findet die I2C-Adresse
  des Displays automatisch (egal ob 0x27, 0x3F, ...).

  Verkabelung (Arduino Uno/Nano): GND -> GND, VCC -> 5V, SDA -> A4, SCL -> A5
  Ist beim Start kein Display angeschlossen, läuft der Rover trotzdem normal weiter.
*/
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// ---------- Einstellungen ----------
// true  = große Laufschrift über beide Zeilen
// false = zwei Seiten, die sich abwechseln
const bool LAUFSCHRIFT = true;

const unsigned long LAUFSCHRIFT_TEMPO_MS = 200;      // Zeit pro Schritt: kleiner = schneller
const unsigned long SEITEN_WECHSEL_MS = 3000;        // so lange bleibt eine Seite stehen
const unsigned long SEITEN_AKTUALISIERUNG_MS = 250;  // so oft werden die Werte neu geschrieben

hd44780_I2Cexp lcd;  // findet Adresse und Anschlussbelegung des Displays selbst
bool lcdVorhanden = false;
unsigned long letzteAnzeige = 0;
int laufPosition = 0;

// ---------- Bausteine für die große Schrift ----------
// Jedes große Zeichen ist 2 Zeilen hoch und aus diesen 8 selbst gezeichneten
// Bausteinen zusammengesetzt (mehr eigene Zeichen kann das Display nicht speichern).
const byte LO = 0;     // Ecke links oben (abgerundet)
const byte OB = 1;     // Balken oben
const byte RO = 2;     // Ecke rechts oben (abgerundet)
const byte LU = 3;     // Ecke links unten (abgerundet)
const byte UB = 4;     // Balken unten
const byte RU = 5;     // Ecke rechts unten (abgerundet)
const byte OM = 6;     // Balken oben + Mitte
const byte MU = 7;     // Balken Mitte + unten
const byte VV = 255;   // volles Kästchen (im Display eingebaut)
const byte LE = ' ';   // leeres Kästchen
const byte GRAD = 0xDF;  // Grad-Zeichen ° (im Display eingebaut)

// Pixelmuster der 8 Bausteine (je 5 Pixel breit, 8 Pixel hoch)
byte BAUSTEINE[8][8] = {
  {0b00111, 0b01111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111},  // LO
  {0b11111, 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000},  // OB
  {0b11100, 0b11110, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111},  // RO
  {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b01111, 0b00111},  // LU
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111},  // UB
  {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11110, 0b11100},  // RU
  {0b11111, 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111},  // OM
  {0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111},  // MU
};

// Ein großes Zeichen: welches Zeichen, wie viele Spalten breit, Bausteine oben und unten
struct GrossesZeichen {
  char zeichen;
  byte breite;
  byte oben[5];
  byte unten[5];
};

// Die Tabelle liegt im Flash-Speicher (PROGMEM), um Arbeitsspeicher zu sparen.
// Deshalb wird sie unten mit pgm_read_byte() gelesen.
const GrossesZeichen SCHRIFT[] PROGMEM = {
  {'0', 3, {LO, OB, RO}, {LU, UB, RU}},
  {'1', 3, {OB, RO, LE}, {UB, VV, UB}},
  {'2', 3, {OM, OM, RO}, {LU, UB, UB}},
  {'3', 3, {OM, OM, RO}, {MU, MU, RU}},
  {'4', 3, {LU, UB, VV}, {LE, LE, VV}},
  {'5', 3, {VV, OM, OM}, {MU, MU, RU}},
  {'6', 3, {LO, OM, OM}, {LU, MU, RU}},
  {'7', 3, {OB, OB, RO}, {LE, LE, VV}},
  {'8', 3, {LO, OM, RO}, {LU, MU, RU}},
  {'9', 3, {LO, OM, RO}, {LE, LE, VV}},
  {'A', 3, {LO, OM, RO}, {VV, LE, VV}},
  {'C', 3, {LO, OB, OB}, {LU, UB, UB}},
  {'D', 3, {VV, OB, RO}, {VV, UB, RU}},
  {'E', 3, {VV, OM, OM}, {VV, MU, MU}},
  {'F', 3, {VV, OM, OM}, {VV, LE, LE}},
  {'G', 3, {LO, OB, OB}, {LU, UB, RU}},
  {'H', 3, {VV, UB, VV}, {VV, LE, VV}},
  {'I', 3, {OB, VV, OB}, {UB, VV, UB}},
  {'L', 3, {VV, LE, LE}, {VV, UB, UB}},
  {'M', 5, {VV, OB, VV, OB, VV}, {VV, LE, VV, LE, VV}},
  {'N', 3, {VV, OB, RO}, {VV, LE, VV}},
  {'P', 3, {VV, OM, RO}, {VV, LE, LE}},
  {'R', 3, {VV, OM, RO}, {VV, LO, LE}},
  {'S', 3, {LO, OM, OM}, {MU, MU, RU}},
  {'T', 3, {OB, VV, OB}, {LE, VV, LE}},
  {'U', 3, {VV, LE, VV}, {LU, UB, RU}},
  {':', 1, {UB}, {UB}},
  {'%', 2, {GRAD, '/'}, {'/', 'o'}},
  {'*', 1, {GRAD}, {LE}},             // * steht im Text für das Grad-Zeichen °
  {'-', 2, {UB, UB}, {LE, LE}},
  {' ', 1, {LE}, {LE}},
  {'_', 3, {LE, LE, LE}, {LE, LE, LE}},  // Leerzeichen, so breit wie eine Ziffer
};
const int SCHRIFT_ANZAHL = sizeof(SCHRIFT) / sizeof(SCHRIFT[0]);

// ---------- Hauptfunktionen ----------

void anzeigeSetup() {
  Wire.begin();
  if (lcd.begin(16, 2) != 0) {  // 0 = Display gefunden
    Serial.println("Kein LCD gefunden - Werte nur im seriellen Monitor");
    return;
  }
  lcdVorhanden = true;
  lcd.backlight();
  for (byte i = 0; i < 8; i++) {
    lcd.createChar(i, BAUSTEINE[i]);
  }
  lcd.clear();
  Serial.println("LCD gefunden");
}

// Zeigt die Werte an. Einfach bei jedem loop()-Durchlauf aufrufen:
// Die Funktion entscheidet selbst, wann das Display neu gezeichnet wird.
void anzeigeUpdate(float feuchtigkeit, float temperatur, int helligkeit, bool geneigt) {
  if (!lcdVorhanden) return;

  unsigned long intervall = LAUFSCHRIFT ? LAUFSCHRIFT_TEMPO_MS : SEITEN_AKTUALISIERUNG_MS;
  if (millis() - letzteAnzeige < intervall) return;
  letzteAnzeige = millis();

  if (LAUFSCHRIFT) {
    char text[64];
    char f[5], t[5], h[5];
    zahlText(f, feuchtigkeit, '_');
    zahlText(t, temperatur, '_');
    zahlText(h, helligkeit, '_');
    snprintf(text, sizeof(text), "FEUCHTE:%s%%   TEMP:%s*C   HELL:%s   NEIGUNG:%s   ",
             f, t, h, geneigt ? "SCHIEF" : "GERADE");
    laufschriftZeichnen(text);
  } else {
    seitenZeichnen(feuchtigkeit, temperatur, helligkeit, geneigt);
  }
}

// ---------- Laufschrift ----------

// Zeichnet einen Schritt der Laufschrift: 16 Spalten ab laufPosition.
void laufschriftZeichnen(const char* text) {
  int laenge = strlen(text);

  // Gesamtbreite des Textes in Spalten (jedes Zeichen + 1 Spalte Abstand)
  int gesamtBreite = 0;
  for (int i = 0; i < laenge; i++) {
    gesamtBreite += zeichenBreite(zeichenSuchen(text[i])) + 1;
  }
  laufPosition %= gesamtBreite;

  // Das Zeichen suchen, bei dem die Anzeige gerade beginnt
  int i = 0;
  int z = zeichenSuchen(text[0]);
  int spalte = laufPosition;
  while (spalte > zeichenBreite(z)) {
    spalte -= zeichenBreite(z) + 1;
    i = (i + 1) % laenge;
    z = zeichenSuchen(text[i]);
  }

  // 16 Spalten zusammensetzen
  byte oben[16], unten[16];
  for (int x = 0; x < 16; x++) {
    oben[x] = zeichenBaustein(z, 0, spalte);
    unten[x] = zeichenBaustein(z, 1, spalte);
    spalte++;
    if (spalte > zeichenBreite(z)) {  // Zeichen fertig -> nächstes Zeichen
      spalte = 0;
      i = (i + 1) % laenge;
      z = zeichenSuchen(text[i]);
    }
  }

  lcd.setCursor(0, 0);
  for (int x = 0; x < 16; x++) lcd.write(oben[x]);
  lcd.setCursor(0, 1);
  for (int x = 0; x < 16; x++) lcd.write(unten[x]);

  laufPosition++;  // beim nächsten Mal eine Spalte weiter -> Text läuft nach links
}

// Gibt die Nummer des Zeichens in der Tabelle zurück, -1 wenn es nicht drin ist
int zeichenSuchen(char c) {
  for (int i = 0; i < SCHRIFT_ANZAHL; i++) {
    if ((char)pgm_read_byte(&SCHRIFT[i].zeichen) == c) return i;
  }
  return -1;
}

byte zeichenBreite(int z) {
  if (z < 0) return 1;  // unbekanntes Zeichen -> schmale Lücke
  return pgm_read_byte(&SCHRIFT[z].breite);
}

// Baustein eines großen Zeichens in Zeile 0 (oben) oder 1 (unten)
byte zeichenBaustein(int z, int zeile, int spalte) {
  if (z < 0 || spalte >= zeichenBreite(z)) return LE;  // Abstand zwischen den Zeichen
  if (zeile == 0) return pgm_read_byte(&SCHRIFT[z].oben[spalte]);
  return pgm_read_byte(&SCHRIFT[z].unten[spalte]);
}

// ---------- Zwei Seiten im Wechsel ----------

void seitenZeichnen(float feuchtigkeit, float temperatur, int helligkeit, bool geneigt) {
  char zeile1[17], zeile2[17], zahl[5];

  if ((millis() / SEITEN_WECHSEL_MS) % 2 == 0) {
    zahlText(zahl, feuchtigkeit, ' ');
    snprintf(zeile1, sizeof(zeile1), "Feuchte: %s %%", zahl);
    zahlText(zahl, temperatur, ' ');
    snprintf(zeile2, sizeof(zeile2), "Temperatur: %s%cC", zahl, (char)GRAD);
  } else {
    zahlText(zahl, helligkeit, ' ');
    snprintf(zeile1, sizeof(zeile1), "Helligkeit: %s", zahl);
    snprintf(zeile2, sizeof(zeile2), "Neigung: %s", geneigt ? "schief" : "gerade");
  }

  zeileSchreiben(0, zeile1);
  zeileSchreiben(1, zeile2);
}

// Schreibt eine Zeile und füllt den Rest mit Leerzeichen (löscht alte Zeichen, ohne zu flackern)
void zeileSchreiben(int zeile, const char* text) {
  lcd.setCursor(0, zeile);
  lcd.print(text);
  for (int i = strlen(text); i < 16; i++) lcd.print(' ');
}

// ---------- Hilfsfunktion ----------

// Schreibt einen Messwert als ganze Zahl in "ziel" (bei Lesefehler "--").
// Einstellige Zahlen bekommen vorne das Füllzeichen, damit sich die Breite nicht ändert.
void zahlText(char* ziel, float wert, char fuellzeichen) {
  if (isnan(wert)) {
    strcpy(ziel, "--");
    return;
  }
  int zahl = (int)(wert + 0.5);  // runden
  if (zahl >= 0 && zahl < 10) {
    snprintf(ziel, 5, "%c%d", fuellzeichen, zahl);
  } else {
    snprintf(ziel, 5, "%d", zahl);
  }
}
