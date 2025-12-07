#include "wifi_manager.h"
#include <HTTPClient.h>

// ------------------------------------------------------------
// Globale Variablen
// ------------------------------------------------------------
WifiConfig wifiConfig;
AsyncWebServer *server = nullptr;
bool apModeActive = false;
bool everConnected = false;
int failedAttempts = 0;
unsigned long wifiStartTime = 0;

// ------------------------------------------------------------
// WLAN starten
// ------------------------------------------------------------
void wifiStart() {
  wifiStartTime = millis();  // Zeit für evtl AP Modus bei Start und nichterreichbarkeit des Boot Wlans
  if (loadWifiConfig()) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiConfig.ssid.c_str(), wifiConfig.password.c_str());
    apModeActive = false;
  } else {
    startAPMode();
  }

  if (server) {
    delete server;
    server = nullptr;
  }
  server = new AsyncWebServer(80);
  setupAPWebServer();
  delay(100);
  server->begin();
}


// ------------------------------------------------------------
// Loop
// -------------------------------------------------------
void wifiLoop() {
  // Wenn verbunden → merken
  if (WiFi.status() == WL_CONNECTED) {
    everConnected = true;
    failedAttempts = 0;
    return;
  }

  // Nur wenn STA aktiv
  if (!apModeActive) {
    static unsigned long lastAttempt = 0;

    // 5-Sekunden Retry
    if (millis() - lastAttempt > 5000) {
      lastAttempt = millis();
      failedAttempts++;

      if (DEBUG_MODE_WIFI) {
        Serial.printf("WLAN getrennt, Versuch %d...\n", failedAttempts);
      }

      WiFi.disconnect();
      WiFi.begin(wifiConfig.ssid.c_str(), wifiConfig.password.c_str());
    }

    // ⏳ AP-Fallback nach 120 Sekunden ohne je verbunden zu sein
    if (!everConnected && (millis() - wifiStartTime > WIFI_AP_FALLBACK_TIMEOUT)) {
      if (DEBUG_MODE_WIFI) {
        Serial.println("⏳ Timeout: 120 Sekunden ohne Verbindung → Starte AP-Modus");
      }
      startAPMode();
    }
  }
}


bool wifiIsConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String getConnectedSSID() {
  return WiFi.status() == WL_CONNECTED ? WiFi.SSID() : "";
}

// ------------------------------------------------------------
// AP-Modus starten (festes Passwort aus Config.h)
// ------------------------------------------------------------
void startAPMode() {
  apModeActive = true;
  WiFi.mode(WIFI_AP);

  bool ok = WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, AP_HIDDEN, AP_MAX_CONN);
  if (!ok) {
    if (DEBUG_MODE_WIFI) Serial.println("❌ Fehler beim Start des AP-Modus!");
    return;
  }

  IPAddress ip = WiFi.softAPIP();

  if (DEBUG_MODE_WIFI) {
    Serial.printf("AP gestartet. IP: %s\n", ip.toString().c_str());
    Serial.printf("AP-Status: %d\n", WiFi.status());
    Serial.printf("📡 AP aktiv: %s\n", AP_SSID);
    Serial.printf("➡ Passwort: %s\n", AP_PASSWORD);
    Serial.printf("➡ IP: %s\n", ip.toString().c_str());
  }
}

// ------------------------------------------------------------
// AP Webserver Setup (korrigiert für AsyncWebServer* server)
// ------------------------------------------------------------
void setupAPWebServer() {
  // Root / config.html aus LittleFS
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/config.html")) {
      request->send(LittleFS, "/config.html", "text/html");
    } else {
      request->send(500, "text/plain", "config.html fehlt");
    }
  });

  // Scan für WLANs
  server->on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; i++) {
      json += "\"" + WiFi.SSID(i) + "\"";
      if (i < n - 1) json += ",";
    }
    json += "]";
    request->send(200, "application/json", json);
  });

  // Save: wir erwarten JSON im Body -> on(..., HTTP_POST, onRequest, onUpload, onBody)
  // onRequest kann leer bleiben (hier: send z.B. 200), onUpload NULL, onBody = body-handler
  server->on(
    "/save", HTTP_POST,
    [](AsyncWebServerRequest *request) { /* onRequest - optional */ },
    nullptr,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      // Directly forward to your implementation which must match this signature:
      handleSaveConfigRequest(request, data, len, index, total);
    });

  // Mastdaten als JSON
  server->on("/data.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    // wenn du einen handler mit Async signature hast:
    sendMastData(request);
  });

  // Optional: einfache POST endpoint falls Mast sendet x-www-form-urlencoded (nicht JSON)
  // server->on("/mastdata", HTTP_POST, [](AsyncWebServerRequest *request){ /*...*/ });

  // Falls du noch andere Endpunkte brauchst, hier ergänzen...
}


// ------------------------------------------------------------
// WLAN Scan
// ------------------------------------------------------------
void handleWifiScanRequest(AsyncWebServerRequest *request) {
  int n = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < n; i++) {
    json += "\"" + WiFi.SSID(i) + "\"";
    if (i < n - 1) json += ",";
  }
  json += "]";
  request->send(200, "application/json", json);
}

// ------------------------------------------------------------
// Config speichern
// ------------------------------------------------------------
void handleSaveConfigRequest(AsyncWebServerRequest *request,
                             uint8_t *data, size_t len,
                             size_t index, size_t total) {
  // Wir verarbeiten nur vollständige Bodies
  if (index != 0 || (index + len) != total) {
    // not complete yet
    return;
  }
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, data, len);
  if (err) {
    request->send(400, "text/plain", "JSON ungültig");
    return;
  }

  if (DEBUG_MODE_WIFI)Serial.println("JSON OK → Werte übernehmen…");

  wifiConfig.ssid = doc["ssid"].as<String>();
  wifiConfig.password = doc["password"].as<String>();
  wifiConfig.port = doc["port"] | 8080;
  wifiConfig.windSpeedIntervalMs = doc["windSpeedInterval"] | 250;
  wifiConfig.windDirIntervalMs = doc["windDirInterval"] | 250;
  wifiConfig.gpsIntervalMs = doc["gpsInterval"] | 250;
  wifiConfig.sendIntervalMs = doc["sendInterval"] | 500;

  saveConfig();
  request->send(200, "text/plain", "Gespeichert. Neustart...");
  delay(300);
  ESP.restart();
}

// ------------------------------------------------------------
// Config laden/speichern
// ------------------------------------------------------------
bool loadWifiConfig() {
  if (!LittleFS.exists("/config.json")) return false;
  File f = LittleFS.open("/config.json", "r");
  if (!f) return false;

  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, f)) {
    f.close();
    return false;
  }
  f.close();

  wifiConfig.ssid = doc["ssid"].as<String>();
  wifiConfig.password = doc["password"].as<String>();
  wifiConfig.port = doc["port"] | 8080;
  wifiConfig.windSpeedIntervalMs = doc["windSpeedInterval"] | 250;
  wifiConfig.windDirIntervalMs = doc["windDirInterval"] | 250;
  wifiConfig.gpsIntervalMs = doc["gpsInterval"] | 250;
  wifiConfig.sendIntervalMs = doc["sendInterval"] | 500;

  return wifiConfig.ssid.length() > 0;
}

// ------------------------------------------------------------
// Mastdaten JSON
// ------------------------------------------------------------
void sendMastData(AsyncWebServerRequest *request) {
  StaticJsonDocument<512> doc;
  //Wind Daten
  doc["winddir"] = mastdaten.winddir_gemessen;
  doc["windspeed"] = mastdaten.windspeed_gemessen;
  //GPS Daten
  doc["lat"] = mastdaten.gps_lat;
  doc["lon"] = mastdaten.gps_lon;
  doc["speed"] = mastdaten.gps_speed;
  doc["kurs"] = mastdaten.gps_kurs;
  doc["sats"] = mastdaten.gps_sats;
  doc["hdop"] = mastdaten.gps_hdop;

  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
}

// ------------------------------------------------------------
// Mast → Boot ESP senden
// ------------------------------------------------------------
void sendDataToBoot() {
  if (!wifiIsConnected()) {
    if (DEBUG_MODE_WIFI) Serial.println("⚠ WLAN nicht verbunden, Daten werden nicht gesendet");
    return;
  }

  WiFiClient client;
  HTTPClient http;

  const char *url = "http://192.168.4.1/mastdata";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

  // JSON-Puffer 512 Byte
  StaticJsonDocument<512> doc;

  // Winddaten
  doc["winddir_gemessen"] = mastdaten.winddir_gemessen;
  doc["windspeed_gemessen"] = mastdaten.windspeed_gemessen;

  // GPS Daten
  doc["gps_lat"] = mastdaten.gps_lat;
  doc["gps_lon"] = mastdaten.gps_lon;
  doc["gps_speed"] = mastdaten.gps_speed;
  doc["gps_kurs"] = mastdaten.gps_kurs;
  doc["gps_sats"] = mastdaten.gps_sats;
  doc["gps_hdop"] = mastdaten.gps_hdop;

  // Datum & Zeit
  doc["gps_jahr"] = mastdaten.gps_jahr;
  doc["gps_monat"] = mastdaten.gps_monat;
  doc["gps_tag"] = mastdaten.gps_tag;
  doc["gps_stunde"] = mastdaten.gps_stunde;
  doc["gps_minute"] = mastdaten.gps_minute;
  doc["gps_sekunde"] = mastdaten.gps_sekunde;

  String body;
  if (serializeJson(doc, body) == 0) {
    if (DEBUG_MODE_WIFI) Serial.println("❌ Fehler: JSON konnte nicht erstellt werden");
    http.end();
    return;
  }

  if (DEBUG_MODE_WIFI) Serial.println("➡ Sende JSON an Boot-ESP32:");
  if (DEBUG_MODE_WIFI) Serial.println(body);

  int httpCode = http.POST(body);
  if (httpCode > 0) {
    if (DEBUG_MODE_WIFI) Serial.printf("✅ HTTP-POST erfolgreich: Code %d\n", httpCode);
  } else {
    if (DEBUG_MODE_WIFI) Serial.printf("❌ Fehler beim HTTP-POST: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
