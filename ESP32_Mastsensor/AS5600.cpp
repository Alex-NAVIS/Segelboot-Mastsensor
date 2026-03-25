/* ==============================================================================================================================================================================
// Windrichtung.cpp – AS5600 Magnet-Encoder für Windrichtung
1. Physische Datenbeschaffung
Der AS5600 misst das Magnetfeld eines rotierenden Magneten mit 12-Bit Auflösung (4096 Schritte). Da der Sensor am Masttop sitzt, nutzt das Modul eine reduzierte 
I2C-Geschwindigkeit (100kHz), um Signalverluste durch lange Kabelwege zu vermeiden. Eine aktive Fehlerprüfung (0xFFFF) verhindert, dass Kabelwackler als falsche 
Winddaten interpretiert werden.
2. Die Bug-Referenz (Calibration & Offset)
Da der Sensor mechanisch selten perfekt auf 0° (nach vorne) ausgerichtet eingebaut werden kann, arbeitet das Modul mit einem Software-Offset.
Bei der Kalibrierung wird die aktuelle Position der Windfahne als "Nullpunkt" (Bug) definiert.
Dieser Wert wird dauerhaft im Dateisystem (LittleFS) gespeichert, sodass die Windfahne auch nach einem Stromausfall weiß, wo "vorne" ist.
3. Der intelligente Filter (Weighted Moving Average)
Windfahnen neigen dazu, durch Schiffsbewegungen und Verwirbelungen im Masttop extrem zu "zittern". Ein einfacher Mittelwert würde jedoch am Umschlagpunkt (359° auf 0°) versagen.
Vektorberechnung: Das Modul zerlegt jeden Winkel in seine Sinus- und Cosinus-Anteile. In dieser mathematischen Form existiert kein Sprung bei 360°, wodurch eine saubere 
Mittelwertbildung über den Nordpunkt hinweg möglich wird.
Gewichteter Ringspeicher: Es werden die letzten 5 Messwerte gespeichert. Durch die Gewichtung (weights) erhält der aktuellste Wert die höchste Priorität (40%). Das Ergebnis 
ist ein extrem ruhiges Signal am Plotter, das dennoch ohne spürbare Verzögerung auf echte Kursänderungen oder Böen reagiert.
4. Datenbereitstellung
Das Endergebnis wird in der zentralen Struktur mastdaten.winddir_gemessen abgelegt und steht somit dem restlichen System (WLAN-Versand, NMEA-Konvertierung) als stabiler, 
korrigierter Wert zur Verfügung.
Soll ich dir noch ein kleines Tool schreiben, mit dem du die Rohwerte und gefilterten Werte grafisch im Serial Plotter vergleichen kannst?
==============================================================================================================================================================================*/

#include "AS5600.h"
#include "Mast_Data.h"
#include "Config.h"
#include <Wire.h>
#include <math.h>

// Standard I2C-Adresse des AS5600
#define AS5600_ADDR 0x36
#define AS5600_REG_RAW_ANGLE 0x0C 

// Ringspeicher für 5 Werte
#define FILTER_SIZE 5
static float ringBuffer[FILTER_SIZE] = {0};
static int bufferIndex = 0;
static bool bufferFull = false;

// Gewichtung: Ältester Wert (Index 0) bis neuester Wert (Index 4)
// Summe muss 1.0 ergeben. Der neueste Wert zählt hier am meisten (40%).
static const float weights[FILTER_SIZE] = { 0.05f, 0.10f, 0.20f, 0.25f, 0.40f };

/**
 * @brief Hilfsfunktion: Liest den 12-Bit Rohwert sicher über I2C aus.
 * 
 * @details Adressiert das RAW_ANGLE Register des AS5600. Die Funktion prüft 
 * aktiv den I2C-Status, um bei Kabelproblemen am Mast keine Fehlwerte zu liefern.
 * 
 * @return uint16_t Rohwert (0-4095) oder 0xFFFF bei Kommunikationsfehler.
 */
static uint16_t readAS5600Raw() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(AS5600_REG_RAW_ANGLE);

  if (Wire.endTransmission(false) != 0) {
    if (DEBUG_MODE_WINDDIR) Serial.println("[AS5600] FEHLER: Sensor nicht am I2C-Bus!");
    return 0xFFFF;
  }

  if (Wire.requestFrom(AS5600_ADDR, (uint8_t)2) == 2) {
    uint16_t high = Wire.read();
    uint16_t low = Wire.read();
    return ((high << 8) | low) & 0x0FFF;
  }
  return 0xFFFF;
}

/**
 * @brief Initialisiert den I2C-Bus für den Windrichtungssensor.
 * 
 * @details Setzt die Taktrate auf 100kHz (Standard Mode). Dies ist robuster gegenüber 
 * Leitungskapazitäten und elektromagnetischen Störungen, die bei langen Kabeln 
 * im Masttop auftreten können.
 */
void setupWindrichtung() {
  Wire.begin(WIND_I2C_SDA, WIND_I2C_SCL, 100000); // 100kHz für lange Leitungen
  if (DEBUG_MODE_WINDDIR) {
    Serial.printf("[AS5600] Init – SDA=%d SCL=%d\n", WIND_I2C_SDA, WIND_I2C_SCL);
  }
}

/**
 * @brief Hauptfunktion zur Messung, Filterung und Korrektur der Windrichtung.
 * 
 * @details Diese Funktion führt vier zentrale Schritte aus:
 * 1. **Rohwert-Korrektur**: Wandelt den 12-Bit-Wert in Grad um und wendet den 
 *    WIND_OFFSET (Bug-Ausrichtung) an.
 * 2. **Ringspeicher-Management**: Speichert den neuen Wert in einem 5-Elemente-Puffer.
 * 3. **Vektor-basierte Filterung**: Da ein normaler Mittelwert am 0°/360°-Umschlagpunkt 
 *    scheitert (Durchschnitt von 359° und 1° wäre fälschlicherweise 180°), werden 
 *    die Winkel in Sinus- (Y) und Cosinus- (X) Komponenten zerlegt.
 * 4. **Gewichtung**: Die Vektoren werden mit zeitlich gestuften Faktoren verrechnet. 
 *    Der neueste Wert hat die höchste Priorität (40%), um eine schnelle Reaktion 
 *    bei Wenden oder Halsen zu gewährleisten.
 * 
 * @note Die Rückumrechnung erfolgt über atan2(), was den Quadranten korrekt 
 * berücksichtigt und den finalen gefilterten Winkel liefert.
 */
void readWindRichtung() {
  uint16_t raw = readAS5600Raw();
  if (raw == 0xFFFF) return;

  // 1. Aktuellen Winkel berechnen (inkl. Bug-Offset)
  float currentAngle = fmodf(((raw * 360.0f) / 4096.0f) - WIND_OFFSET + 360.0f, 360.0f);

  // 2. In Ringspeicher schreiben
  ringBuffer[bufferIndex] = currentAngle;
  bufferIndex = (bufferIndex + 1) % FILTER_SIZE;
  if (bufferIndex == 0) bufferFull = true;

  // 3. Gewichteten Mittelwert über Vektoren (Sin/Cos) berechnen
  // Verhindert Fehler beim Sprung von 360 auf 0 Grad
  float sumX = 0;
  float sumY = 0;
  int count = bufferFull ? FILTER_SIZE : bufferIndex;

  for (int i = 0; i < count; i++) {
    // Index des Werts im Ringspeicher finden (vom ältesten zum neuesten)
    int idx = (bufferIndex - count + i + FILTER_SIZE) % FILTER_SIZE;
    
    float rad = ringBuffer[idx] * (M_PI / 180.0f);
    // Wir nutzen die Gewichte passend zum Alter des Werts
    sumX += cos(rad) * weights[i];
    sumY += sin(rad) * weights[i];
  }

  // 4. Zurück in Winkel wandeln (atan2 liefert -PI bis PI)
  float filteredAngle = atan2(sumY, sumX) * (180.0f / M_PI);
  if (filteredAngle < 0) filteredAngle += 360.0f;

  // In Mast-Datenstruktur speichern
  mastdaten.winddir_gemessen = filteredAngle;

  if (DEBUG_MODE_WINDDIR) {
    Serial.printf("[WIND] Raw=%4u | Ist=%5.1f° | Gefeiltet=%5.1f°\n", raw, currentAngle, filteredAngle);
  }
}

/**
 * @brief Kalibriert die Windfahne auf die Vorausrichtung (Bug = 0°).
 * 
 * Diese Funktion liest den aktuellen mechanischen Ist-Zustand des Magnet-Sensors 
 * aus und setzt diesen als neuen Referenzpunkt (Nullpunkt) für das Schiff.
 * 
 * Ablauf:
 * 1. Abfrage des 12-Bit Rohwerts (0-4095) vom AS5600 über I2C.
 * 2. Validierung: Nur bei erfolgreicher Kommunikation (nicht 0xFFFF) wird fortgefahren.
 * 3. Berechnung: Der Rohwert wird in einen Gradwert (0-359.9°) umgerechnet und 
 *    als globaler WIND_OFFSET gespeichert.
 * 4. Persistenz: Der neue Offset wird sofort via LittleFS in der 'config.json' 
 *    gesichert, damit er nach einem Neustart erhalten bleibt.
 * 5. Filter-Reset: Der Ringspeicher (Moving Average) wird zurückgesetzt (Index 0, 
 *    Full-Flag false). Dies verhindert, dass alte Messwerte vor der Kalibrierung 
 *    das neue "Null-Ergebnis" verfälschen und sorgt für eine sofortige 0°-Anzeige.
 * 6. Debug: Erfolgsmeldung mit dem neuen berechneten Offset auf der seriellen Konsole.
 */
void calibrateWindToNorth() {
  uint16_t raw = readAS5600Raw();
  if (raw != 0xFFFF) {
    WIND_OFFSET = (raw * 360.0f) / 4096.0f;
    saveConfig();
    // Nach Kalibrierung Ringspeicher kurz resetten, damit sofort 0 angezeigt wird
    bufferFull = false;
    bufferIndex = 0;
    
    if (DEBUG_MODE_WINDDIR) {
      Serial.printf("[AS5600] KALIBRIERT! Offset: %.2f°\n", WIND_OFFSET);
    }
  }
}
