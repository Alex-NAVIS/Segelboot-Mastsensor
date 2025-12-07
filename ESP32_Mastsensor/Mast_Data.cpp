#include "Mast_Data.h"

Mast_Sensoren mastdaten = {
  // Windsensoren
  .winddir_gemessen = 0.0f,
  .windspeed_gemessen = 0.0f,

  // GPS-Daten
  .gps_lat = 0.0,
  .gps_lon = 0.0,
  .gps_speed = 0.0,
  .gps_kurs = 0.0,
  .gps_sats = 0,
  .gps_hdop = 99.9f,

  // GPS Datum/Zeit (ungültig = 0)
  .gps_jahr = 0,
  .gps_monat = 0,
  .gps_tag = 0,
  .gps_stunde = 0,
  .gps_minute = 0,
  .gps_sekunde = 0,
};
