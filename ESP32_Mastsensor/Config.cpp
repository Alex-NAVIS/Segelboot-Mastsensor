#include "Config.h"
#include <ArduinoJson.h>

#define DEFAULT_PORT          8080
#define DEFAULT_WIND_INTERVAL 250
#define DEFAULT_GPS_INTERVAL  250
#define DEFAULT_SEND_INTERVAL 500
#define DEFAULT_SSID          ""
#define DEFAULT_PASSWORD      ""

// ------------------------------------------------------------
// Config laden
// ------------------------------------------------------------
void loadConfig() {
    if (!LittleFS.begin()) {
        Serial.println("LittleFS konnte nicht gestartet werden! -> Format?");
        LittleFS.format();
        ESP.restart();
    }

    if (LittleFS.exists("/config.json")) {
        File f = LittleFS.open("/config.json", "r");
        if (!f) {
            Serial.println("Fehler beim Öffnen von config.json");
        } else {
            StaticJsonDocument<512> doc;
            DeserializationError error = deserializeJson(doc, f);
            f.close();
            if (!error) {
                wifiConfig.ssid            = doc["ssid"].as<String>();
                wifiConfig.password        = doc["password"].as<String>();
                wifiConfig.port            = doc["port"] | DEFAULT_PORT;
                wifiConfig.windSpeedIntervalMs = doc["windSpeedInterval"] | DEFAULT_WIND_INTERVAL;
                wifiConfig.windDirIntervalMs   = doc["windDirInterval"]   | DEFAULT_WIND_INTERVAL;
                wifiConfig.gpsIntervalMs       = doc["gpsInterval"]       | DEFAULT_GPS_INTERVAL;
                wifiConfig.sendIntervalMs      = doc["sendInterval"]      | DEFAULT_SEND_INTERVAL;

                Serial.println("✅ Config geladen:");
                Serial.println("SSID: " + wifiConfig.ssid);
            } else {
                Serial.println("Fehler beim Parsen der config.json, Defaultwerte werden genutzt");
            }
        }
    } else {
        Serial.println("Keine Config vorhanden, Defaultwerte werden genutzt");
        wifiConfig.ssid            = DEFAULT_SSID;
        wifiConfig.password        = DEFAULT_PASSWORD;
        wifiConfig.port            = DEFAULT_PORT;
        wifiConfig.windSpeedIntervalMs = DEFAULT_WIND_INTERVAL;
        wifiConfig.windDirIntervalMs   = DEFAULT_WIND_INTERVAL;
        wifiConfig.gpsIntervalMs       = DEFAULT_GPS_INTERVAL;
        wifiConfig.sendIntervalMs      = DEFAULT_SEND_INTERVAL;
    }
}

// ------------------------------------------------------------
// Config speichern
// ------------------------------------------------------------
void saveConfig() {
    StaticJsonDocument<512> doc;
    doc["ssid"]             = wifiConfig.ssid;
    doc["password"]         = wifiConfig.password;
    doc["port"]             = wifiConfig.port;
    doc["windSpeedInterval"] = wifiConfig.windSpeedIntervalMs;
    doc["windDirInterval"]   = wifiConfig.windDirIntervalMs;
    doc["gpsInterval"]       = wifiConfig.gpsIntervalMs;
    doc["sendInterval"]      = wifiConfig.sendIntervalMs;

    File f = LittleFS.open("/config.json", "w");
    if (!f) {
        Serial.println("Fehler beim Öffnen von config.json zum Schreiben");
        return;
    }

    if (serializeJson(doc, f) == 0) {
        Serial.println("Fehler beim Schreiben von config.json");
    }
    f.close();
    Serial.println("✅ Config gespeichert");
}
