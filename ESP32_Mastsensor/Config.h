#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include "wifi_manager.h"

// ==========================================================
// 1. Feste AP-Konfiguration, nicht veränderbar
// ==========================================================

#define AP_SSID "MastSetup"
#define AP_PASSWORD "mast1234"
#define AP_CHANNEL 3
#define AP_HIDDEN 0
#define AP_MAX_CONN 4

// ==========================================================
// 2. Standard-WLAN-Client-Konfiguration (änderbar über config.json)
// ==========================================================
struct WifiConfig {
  String ssid;
  String password;
  uint16_t port = 80;

  unsigned long windSpeedIntervalMs = 500;
  unsigned long windDirIntervalMs = 500;
  unsigned long gpsIntervalMs = 900;
  unsigned long sendIntervalMs = 1000;
};

extern WifiConfig wifiConfig;

void loadConfig();
void saveConfig();

// ==========================================================
// 3. Debug-Einstellungen
// ==========================================================
#define DEBUG_MODE false
#define DEBUG_MODE_WIFI false
#define DEBUG_MODE_GPS false
#define DEBUG_MODE_WINDSPEED false
#define DEBUG_MODE_WINDRICHTUNG false
#define DEBUG_MODE_WINDDIR true

// ==========================================================
// 4. Zeit in ms bis der Mast-ESP bei fehlender WLAN-Verbindung in den AP-Modus geht
// ==========================================================
#define WIFI_AP_FALLBACK_TIMEOUT 90000  // 90 Sekunden

// ==========================================================
// 5. GPS-Konfiguration (UART)
// ==========================================================
#define GPS_SERIAL_PORT Serial1
#define GPS_BAUDRATE 9600

// GPS Pins
#define GPS_RX_PIN 5   // GPS → ESP32
#define GPS_TX_PIN 17  // ESP32 → GPS

// GPS Vorhersage-Konstante
// GPS liefert nur 1 Hz. Dadurch entsteht besonders beim Losfahren aus dem Hafen ein "Nachlaufen" der Position (bis zu 5–10 Sekunden Verzögerung).
// Wir bilden Bewegungsvektoren der letzten Sekunden und schätzen dann ab, wo sich das Boot nach dieser Zeit befinden würde. Dadurch reagieren Position,
// Kurs und Geschwindigkeit deutlich schneller.

// Vorhersage/Filter-Einstellungen (in Sekunden)
#define GPS_PREDICT_S 1.0f         // wie viele Sekunden in die Zukunft prognostizieren
#define GPS_MIN_VALID_SPEED 0.13f  // kn — unterhalb: als "stillstehend" behandeln
#define GPS_HDOP_MAX_WEIGHT 20.0f  // HDOP höheren als das geben sehr kleinen weight

// GPS AHRS true Stabilisierung false reine GPS Daten
#define GPS_AHRS_SYSTEM true

// ==========================================================
// 6. Rotary Encoder (Windgeschwindigkeit)
// ==========================================================
#define ENCODER_PIN 14

// ==========================================================
// 7. Wind Richtung AS5600
// ==========================================================
#define WIND_I2C_SDA 18  // Default – kann in Config überschrieben werden
#define WIND_I2C_SCL 19