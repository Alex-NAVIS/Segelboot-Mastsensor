#ifndef MAST_DATA_H
#define MAST_DATA_H


struct Mast_Sensoren {
  // --------------------------------------------------------
  // Windsensoren (AS5600 & Encoder)
  // --------------------------------------------------------
  float winddir_gemessen;  // Grad 0..360
  float windspeed_gemessen;  // Impulse pro Sekunde Rotary Encoder

  // --------------------------------------------------------
  // GPS-Daten (TinyGPS++)
  // --------------------------------------------------------
  double gps_lat;    // Geografische Breite (°)
  double gps_lon;    // Geografische Länge (°)
  double gps_speed;  // Geschwindigkeit über Grund (kn/h)
  double gps_kurs;   // Kurs über Grund (°)
  int gps_sats;      // GPS Satanzahl
  float gps_hdop;    // gps hdop Fehlerwert

  // --------------------------------------------------------
  // GPS-Datum & Zeit (aus NMEA)
  // --------------------------------------------------------
  int gps_jahr;     // Jahr (z. B. 2025)
  int gps_monat;    // Monat (1–12)
  int gps_tag;      // Tag (1–31)
  int gps_stunde;   // Stunde (0–23)
  int gps_minute;   // Minute (0–59)
  int gps_sekunde;  // Sekunde (0–59)
};

extern Mast_Sensoren mastdaten;


#endif
