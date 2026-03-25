// ==========================================================
// WindSpeed_Encoder.h
// ----------------------------------------------------------
// Header-Datei für den Windgeschwindigkeitssensor.
// Dieser Sensor misst die Rotationsgeschwindigkeit eines
// Anemometers (Windrad) über einen digitalen Impulsgeber
// (z. B. Reed-Schalter, Hallsensor oder Lichtschranke).
//
// Funktionsprinzip:
//   Jeder Umdrehungsimpuls löst einen Interrupt aus.
//   Im Programm wird die Zeit zwischen den Pulsen gemessen,
//   um daraus die Windgeschwindigkeit zu berechnen.
//
// Anschlussbeispiel (ESP32):
//   Signal-Pin  → Pin 14 (beliebiger GPIO mit Interrupt-Fähigkeit)
//   VCC         → 3.3 V
//   GND         → GND
//
// Siehe Implementierung in WindSpeed_Encoder.cpp für Details zur Berechnung.
// ==========================================================

#ifndef WINDSPEED_ENCODER_H
#define WINDSPEED_ENCODER_H

// ----------------------------------------------------------
// setupWindEncoder()
// ----------------------------------------------------------
// Initialisiert den Interrupt-Pin für das Anemometer.
// Richtet die ISR (Interrupt Service Routine) ein,
// die bei jeder Flanke (Impuls) aufgerufen wird.
// ----------------------------------------------------------
void setupWindEncoder();

// ----------------------------------------------------------
// readWindEncoder()
// ----------------------------------------------------------
// Wird regelmäßig im Hauptprogramm (loop) aufgerufen.
// Berechnet aus der Anzahl der Impulse und der verstrichenen
// Zeit die aktuelle Windgeschwindigkeit.
//
// Ergebnis wird in sensorData.windspeed_gemessen gespeichert.
// ----------------------------------------------------------
void readWindEncoder();
#endif
