// ==========================================================
// Windrichtung.cpp – AS5600 Magnet-Encoder für Windrichtung
// ==========================================================
#include "AS5600.h"
#include "Mast_Data.h"
#include "Config.h"
#include <Wire.h>

// Standard I2C-Adresse des AS5600
#define AS5600_ADDR 0x36
#define AS5600_REG_ANGLE 0x0E   // High + Low Byte


// ----------------------------------------------------------
// Hilfsfunktion:  AS5600 Winkel auslesen
// ----------------------------------------------------------
static uint16_t readAS5600Raw() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(AS5600_REG_ANGLE);
  Wire.endTransmission(false);

  Wire.requestFrom(AS5600_ADDR, 2);
  if (Wire.available() < 2) return 0;

  uint8_t high = Wire.read();
  uint8_t low  = Wire.read();
  return ((uint16_t)high << 8) | low;
}

// ----------------------------------------------------------
// setupWindrichtung()
// ----------------------------------------------------------
void setupWindrichtung() {
  Wire.begin(WIND_I2C_SDA, WIND_I2C_SCL, 400000);

  if (DEBUG_MODE_WINDDIR) {
    Serial.print("[AS5600] Init – SDA=");
    Serial.print(WIND_I2C_SDA);
    Serial.print(" SCL=");
    Serial.println(WIND_I2C_SCL);
  }
}

// ----------------------------------------------------------
// readWindRichtung() – liest Richtung in Grad (0–360)
// ----------------------------------------------------------
void readWindRichtung() {
  uint16_t raw = readAS5600Raw();  // 0–4095

  // 12-bit Wert → 360°
  float angleDeg = (raw * 360.0) / 4096.0;

  // In Mast-Datenstruktur speichern
  mastdaten.winddir_gemessen = angleDeg;

  if (DEBUG_MODE_WINDDIR) {
    Serial.print("[AS5600] raw=");
    Serial.print(raw);
    Serial.print(" -> ");
    Serial.print(angleDeg, 2);
    Serial.println("°");
  }
}
