#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "Config.h"
#include "Mast_Data.h"
#include "AS5600.h"

// ------------------------------------------------------------
// Globale Variablen
// ------------------------------------------------------------
extern AsyncWebServer *server;
extern bool apModeActive;

// ------------------------------------------------------------
// Funktionen
// ------------------------------------------------------------
void wifiStart();
void wifiLoop();
bool wifiIsConnected();
String getConnectedSSID();

bool loadWifiConfig();

void startAPMode();
void setupAPWebServer();

// API Handler
void handleWifiScanRequest(AsyncWebServerRequest *request);
void handleSaveConfigRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void sendMastData(AsyncWebServerRequest *request);

// Listet alle Dateien und Ordner im LittleFS auf
void listDir(fs::FS &fs, String dirname, String &json, bool &first);

// JSON push zum Boot-ESP32
void sendDataToBoot();
