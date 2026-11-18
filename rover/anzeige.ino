/*
  LCD-Display 20x4 (Sunstar 2004A) mit I2C-Modul – zeigt alle Messwerte live an
  ==============================================================================
  Benötigt die Bibliothek "hd44780" von Bill Perry. Die findet die I2C-Adresse
  des Displays automatisch (egal ob 0x27, 0x3F, ...).

  Verkabelung (Arduino Uno/Nano): GND -> GND, VCC -> 5V, SDA -> A4, SCL -> A5
  Ist beim Start kein Display angeschlossen, läuft der Rover trotzdem normal weiter.

  Zwei Anzeige-Arten (unten bei BALKENANZEIGE umschalten):

  Balkenanzeige (true)            Nur Werte (false)
  +--------------------+          +--------------------+
  |Feuchte: 45% ###    |          |Feuchte:    45 %    |
  |Temp:   22°C ###    |          |Temperatur: 22 °C   |
  |Hell:  300lx ###    |          |Helligkeit: 300 lx  |
  |Neigung: gerade ____|          |Neigung:    gerade  |
  +--------------------+          +--------------------+
  Die Balken wachsen live mit den Werten. Fährt der Rover bergauf,
  wird aus der flachen Linie ____ eine Rampe nach oben, bergab eine nach unten.
*/
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// ---------- Einstellungen ----------
// true  = Werte mit Balken, die live mitwachsen, und Neigungs-Symbol
// false = nur die Werte als Text
const bool BALKENANZEIGE = true;

const unsigned long ANZEIGE_INTERVALL_MS = 250;  // so oft wird das Display neu beschrieben

// Höchstwerte = Balken ganz voll
const float FEUCHTIGKEIT_MAX = 100;  // %
const float TEMPERATUR_MAX = 50;     // °C (mehr kann der DHT11 nicht messen)
const float LUX_BALKEN_MAX = 100000; // Lux (pralle Sonne). Der Helligkeits-Balken ist logarithmisch,
                                     // sonst wäre er bei Zimmerlicht (ca. 300 lx) fast leer.

// ---------- Display ----------
const int SPALTEN = 20;
const int ZEILEN = 4;
const int BALKEN_LAENGE = 7;  // Kästchen pro Balken (jedes Kästchen hat 5 Pixel-Spalten)

const byte VOLL = 255;    // volles Kästchen (im Display eingebaut)
const byte GRAD = 0xDF;   // Grad-Zeichen ° (im Display eingebaut)
const byte RAMPE_HOCH = 4;    // eigene Zeichen 4 und 5 = Rampe nach oben (bergauf)
const byte RAMPE_RUNTER = 6;  // eigene Zeichen 6 und 7 = Rampe nach unten (bergab)

hd44780_I2Cexp lcd;  // findet Adresse und Anschlussbelegung des Displays selbst
bool lcdVorhanden = false;
unsigned long letzteAnzeige = 0;

// Eigene Zeichen (je 5 Pixel breit, 8 Pixel hoch). Das Display kann 8 davon speichern.
byte EIGENE_ZEICHEN[8][8] = {
  // 0-3: Balken-Enden, 1 bis 4 Pixel-Spalten gefüllt (5 Spalten = volles Kästchen)
  {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000},
  {0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000},
  {0b11100, 0b11100, 0b11100, 0b11100, 0b11100, 0b11100, 0b11100, 0b11100},
  {0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110},
  // 4-5: Rampe nach oben über 2 Kästchen nebeneinander
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00011, 0b00111, 0b01111, 0b11111},
  {0b00011, 0b00111, 0b01111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111},
  // 6-7: Rampe nach unten über 2 Kästchen nebeneinander
  {0b11000, 0b11100, 0b11110, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111},
  {0b00000, 0b00000, 0b00000, 0b00000, 0b11000, 0b11100, 0b11110, 0b11111},
};

// ---------- Hauptfunktionen ----------

void anzeigeSetup() {
  Wire.begin();
  if (lcd.begin(SPALTEN, ZEILEN) != 0) {  // 0 = Display gefunden
    Serial.println("Kein LCD gefunden - Werte nur im seriellen Monitor");
    return;
  }
  lcdVorhanden = true;
  lcd.backlight();
  for (byte i = 0; i < 8; i++) {
    lcd.createChar(i, EIGENE_ZEICHEN[i]);
  }
  lcd.clear();
  Serial.println("LCD gefunden");
}

// Zeigt die Werte an. Einfach bei jedem loop()-Durchlauf aufrufen:
// Die Funktion entscheidet selbst, wann das Display neu beschrieben wird.
// neigung ist GERADE, STEIGT, SINKT oder UNKLAR (siehe rover.ino)
void anzeigeUpdate(float feuchtigkeit, float temperatur, float lux, int neigung) {
  if (!lcdVorhanden) return;
  if (millis() - letzteAnzeige < ANZEIGE_INTERVALL_MS) return;
  letzteAnzeige = millis();

  if (BALKENANZEIGE) {
    balkenAnzeigen(feuchtigkeit, temperatur, lux, neigung);
  } else {
    werteAnzeigen(feuchtigkeit, temperatur, lux, neigung);
  }
}

// ---------- Balkenanzeige ----------

void balkenAnzeigen(float feuchtigkeit, float temperatur, float lux, int neigung) {
  char text[SPALTEN + 1];
  char zahl[4];  // höchstens 3 Zeichen, z. B. "100" oder "--"

  // Jede Zeile: 13 Zeichen Text, dann 7 Kästchen Balken = 20 Zeichen
  zahlText(zahl, feuchtigkeit);
  snprintf(text, sizeof(text), "Feuchte:%3s%% ", zahl);
  lcd.setCursor(0, 0);
  lcd.print(text);
  balkenSchreiben(feuchtigkeit, FEUCHTIGKEIT_MAX);

  zahlText(zahl, temperatur);
  snprintf(text, sizeof(text), "Temp:   %2s%cC ", zahl, (char)GRAD);
  lcd.setCursor(0, 1);
  lcd.print(text);
  balkenSchreiben(temperatur, TEMPERATUR_MAX);

  snprintf(text, sizeof(text), "Hell:%5ldlx ", (long)(lux + 0.5));
  lcd.setCursor(0, 2);
  lcd.print(text);
  // logarithmisch: 1 lx, 10 lx, 100 lx, 1000 lx, ... wachsen jeweils um gleich viel
  balkenSchreiben(log10(lux + 1), log10(LUX_BALKEN_MAX + 1));

  // Neigung: Text und Symbol in den letzten 4 Kästchen
  //   gerade: ____    steigt: __ + Rampe hoch    sinkt: Rampe runter + __
  snprintf(text, sizeof(text), "Neigung: %-6s ", neigungText(neigung));
  lcd.setCursor(0, 3);
  lcd.print(text);

  byte symbol[4] = {'_', '_', '_', '_'};
  if (neigung == STEIGT) {
    symbol[2] = RAMPE_HOCH;
    symbol[3] = RAMPE_HOCH + 1;
  } else if (neigung == SINKT) {
    symbol[0] = RAMPE_RUNTER;
    symbol[1] = RAMPE_RUNTER + 1;
  } else if (neigung == UNKLAR) {
    for (byte i = 0; i < 4; i++) symbol[i] = ' ';
  }
  for (byte i = 0; i < 4; i++) {
    lcd.write(symbol[i]);
  }
}

// Schreibt einen Balken aus BALKEN_LAENGE Kästchen an die aktuelle Position.
// Jedes Kästchen hat 5 Pixel-Spalten, so wächst der Balken in feinen Schritten.
void balkenSchreiben(float wert, float maximum) {
  int pixel = 0;
  if (!isnan(wert)) {
    pixel = constrain((int)round(wert / maximum * BALKEN_LAENGE * 5), 0, BALKEN_LAENGE * 5);
  }

  for (int i = 0; i < BALKEN_LAENGE; i++) {
    int rest = pixel - i * 5;  // so viele Pixel-Spalten fallen in dieses Kästchen
    if (rest >= 5) {
      lcd.write(VOLL);
    } else if (rest <= 0) {
      lcd.write((byte)' ');
    } else {
      lcd.write((byte)(rest - 1));  // eigenes Zeichen 0-3 = 1-4 Spalten gefüllt
    }
  }
}

// ---------- Nur Werte ----------

void werteAnzeigen(float feuchtigkeit, float temperatur, float lux, int neigung) {
  char text[SPALTEN + 1];
  char zahl[4];  // höchstens 3 Zeichen, z. B. "100" oder "--"

  // Alle Werte beginnen in Spalte 12, damit sie schön untereinander stehen
  // und auch "Helligkeit: 99999 lx" noch in die 20 Zeichen passt
  zahlText(zahl, feuchtigkeit);
  snprintf(text, sizeof(text), "Feuchte:    %s %%", zahl);
  zeileSchreiben(0, text);

  zahlText(zahl, temperatur);
  snprintf(text, sizeof(text), "Temperatur: %s %cC", zahl, (char)GRAD);
  zeileSchreiben(1, text);

  snprintf(text, sizeof(text), "Helligkeit: %ld lx", (long)(lux + 0.5));
  zeileSchreiben(2, text);

  snprintf(text, sizeof(text), "Neigung:    %s", neigungText(neigung));
  zeileSchreiben(3, text);
}

// Schreibt eine Zeile und füllt den Rest mit Leerzeichen (löscht alte Zeichen, ohne zu flackern)
void zeileSchreiben(int zeile, const char* text) {
  lcd.setCursor(0, zeile);
  lcd.print(text);
  for (int i = strlen(text); i < SPALTEN; i++) lcd.print(' ');
}

// ---------- Hilfsfunktion ----------

// Schreibt einen Messwert als gerundete ganze Zahl in "ziel" (bei Lesefehler "--")
void zahlText(char* ziel, float wert) {
  if (isnan(wert)) {
    strcpy(ziel, "--");
  } else {
    snprintf(ziel, 4, "%d", (int)round(wert));
  }
}
