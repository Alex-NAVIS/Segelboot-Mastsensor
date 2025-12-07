// ==========================================================
// WindSpeed_Encoder.cpp (angepasst für Rotary Encoder)
// ----------------------------------------------------------
// Berechnet Windgeschwindigkeit (in Knoten) aus Drehimpulsen
// eines Rotary Encoders mit 600 Schritten pro Umdrehung.
//
// Kalibrierung:
//   2 Umdrehungen/Sekunde entsprechen 28 km/h ≙ 15.12 kn
//   => Faktor = 7.56 kn pro Umdrehung/Sekunde
// ==========================================================

#include "Wind_speed.h"
#include "Mast_Data.h"
#include "Config.h"
#include <Arduino.h>

// ----------------------------------------------------------
// Encoder-Konfiguration
// ----------------------------------------------------------
#define STEPS_PER_REV 600.0         // Impulse pro Umdrehung
#define KNOTS_PER_REV_PER_SEC 7.56  // Kalibrierungsfaktor (kn pro U/s)

// ----------------------------------------------------------
// Globale Variablen
// ----------------------------------------------------------
volatile unsigned long pulseCount = 0;
unsigned long lastReadTime = 0;
unsigned long lastCount = 0;

// ----------------------------------------------------------
// ISR-Prototyp (wichtig für Arduino)
void IRAM_ATTR onEncoderPulse();

// ----------------------------------------------------------
// ISR: Zählt Impulse vom Rotary Encoder
// ----------------------------------------------------------
void IRAM_ATTR onEncoderPulse() {
  pulseCount++;
}

// ----------------------------------------------------------
// setupWindEncoder()
// ----------------------------------------------------------
void setupWindEncoder() {
  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), onEncoderPulse, RISING);

  if (DEBUG_MODE_WINDSPEED) {
    Serial.println("[WindEncoder] Initialisiert (zeitbasierte Messung aktiv)");
  }

  lastReadTime = millis();   // Initialisierung der Zeit
}

// ----------------------------------------------------------
// readWindEncoder() - zeitbasierte Berechnung
// ----------------------------------------------------------
void readWindEncoder() {
  unsigned long now = millis();
  unsigned long dt = now - lastReadTime;   // Zeit seit letztem Aufruf in ms

  if (dt < 5) return;  // Sicherheit: extrem kleine Intervalle ignorieren
  lastReadTime = now;

  // Atomar Pulse auslesen
  noInterrupts();
  unsigned long current = pulseCount;
  interrupts();

  unsigned long diff = current - lastCount;
  lastCount = current;

  double dtSec = dt / 1000.0;  // Zeit in Sekunden

  // Umdrehungen pro Sekunde
  double revsPerSec = (diff / STEPS_PER_REV) / dtSec;

  // Windgeschwindigkeit in Knoten
  double windKnots = revsPerSec * KNOTS_PER_REV_PER_SEC;

  // Speichern in Mast-Datenstruktur
  mastdaten.windspeed_gemessen = windKnots;

  if (DEBUG_MODE_WINDSPEED) {
    Serial.print("[WindEncoder] Pulses=");
    Serial.print(diff);
    Serial.print(" dt=");
    Serial.print(dt);
    Serial.print("ms | rps=");
    Serial.print(revsPerSec, 3);
    Serial.print(" | kn=");
    Serial.println(windKnots, 2);
  }
}
