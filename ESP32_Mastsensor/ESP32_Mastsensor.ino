#include <Arduino.h>
#include <LittleFS.h>

#include "Config.h"
#include "wifi_manager.h"
#include "Mast_Data.h"
#include "GPS.h"
#include "Wind_speed.h"
#include "AS5600.h"

unsigned long lastSend = 0;
unsigned long lastgps = 10;
unsigned long lastwindspeed = 20;
unsigned long lastwinddir = 30;
unsigned long lastwindrichtung = 40;

void setup() {
  Serial.begin(115200);
  delay(300);

  if (DEBUG_MODE) Serial.println("\n--- Mast Modul Start ---");

  // --------------------------------------------------------
  // LITTLEFS starten
  // --------------------------------------------------------
  if (!LittleFS.begin()) {
    if (DEBUG_MODE) Serial.println("LittleFS konnte nicht gestartet werden! -> Format?");
    LittleFS.format();
    ESP.restart();
  }

  // --------------------------------------------------------
  // CONFIG (global) laden
  // NICHT WLAN! Das macht wifi_manager separat.
  // --------------------------------------------------------
  loadConfig();

  // --------------------------------------------------------
  // WLAN starten (Client → Boot-ESP oder AP-Fallback)
  // --------------------------------------------------------
  wifiStart();

  // --------------------------------------------------------
  // Setup GPS Sensor UART Serial1
  // --------------------------------------------------------
  setup_GPS();

  // --------------------------------------------------------
  // Setup Rotary Encoder Windgeschwindigkeit
  // --------------------------------------------------------
  setupWindEncoder();

  // --------------------------------------------------------
  // Setup AS5600 Windrichtung
  // --------------------------------------------------------
  setupWindrichtung();

  if (DEBUG_MODE) Serial.println("--- Setup fertig ---");
}

void loop() {
  wifiLoop();  // Reconnect + Server-Service

  if (millis() - lastgps >= wifiConfig.gpsIntervalMs) {
    lastgps = millis();
    read_GPS();
  }

  if (millis() - lastwindspeed >= wifiConfig.windSpeedIntervalMs) {
    lastwindspeed = millis();
    readWindEncoder();
  }

  // --------------------------------------------------------
  // Daten zyklisch senden
  // --------------------------------------------------------
  if (millis() - lastwindrichtung >= wifiConfig.windDirIntervalMs) {
    lastwindrichtung = millis();
    readWindRichtung();
  }

  // --------------------------------------------------------
  // Daten zyklisch senden
  // --------------------------------------------------------
  if (millis() - lastSend >= wifiConfig.sendIntervalMs) {
    lastSend = millis();
    sendDataToBoot();
  }

  
}
