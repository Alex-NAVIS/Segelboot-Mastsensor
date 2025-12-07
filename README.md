Segelboot‑Mastsensor

NAVIS – Mastsensor‑Modul
Dies ist unser gemeinsames Mastsensor‑Modul für das Segelboot — so gebaut, dass Mast‑ und Sensordaten zuverlässig erfasst, gespeichert und abfragbar sind.

🚀 Was der Mastsensor macht

Der Sensor läuft auf einem ESP32 an Bord.

Er kombiniert mehrere Sensoren — typischerweise eine IMU (9 Achsen: Beschleunigung, Gyroskop, Magnetometer) für Roll, Pitch und Heading — und ggf. weitere Sensoren am Mast.

Der ESP32 stellt einen WLAN‑Access‑Point (AP‑Modus) bereit — damit kann man sich direkt mit Smartphone/Laptop verbinden.

Eine Web‑UI (HTML/CSS/JS) läuft auf dem Gerät und liegt im Flash‑Filesystem (z. B. mit LittleFS), um Mast‑ und Sensordaten übersichtlich anzuzeigen und zu konfigurieren. 
GitHub
+1

🛠️ Setup & Flash‑Workflow

Firmware mit ESP32‑Toolchain (Arduino IDE oder PlatformIO) kompilieren.

Web‑UI‑Assets (Ordner web/) mittels LittleFS‑Uploader (oder Plattform‑spezifischem Tool) auf den ESP32 flashen. Tools wie arduino‑littlefs‑upload erleichtern das — sehr praktisch bei ESP32‑Projekten. 
GitHub
+1

Nach dem Start erzeugt der ESP32 einen WLAN‑AP — mit Smartphone oder Laptop verbinden, Web‑UI im Browser öffnen.

Sensoren kalibrieren, Werte prüfen, Mastdaten live anzeigen lassen.

⚠️ Hinweise & „Insider“-Tipps

LittleFS vs. SPIFFS: Wir empfehlen LittleFS — moderner, stabiler und mit besserer Datei-/Ordnerstruktur. 
GitHub
+1

Wenn LittleFS Partition oder Upload fehlschlägt — evtl. vorher Flash löschen, sauber neu flashen. Manche Bugs oder Inkonsistenzen mit OTA / Dateisystem‑Uploads sind bekannt. 
GitHub
+1

Für die IMU: Kalibrierung ist entscheidend. Magnetometer / Kompasswerte reagieren auf Metall, elektrisches Rauschen oder Nähe zu Boot‑Metall. Offset + ggf. Hard‑/Soft‑Iron‑Korrektur speichern.

Web‑Server asynchron laufen lassen (z. B. mit asynchroner Webserver‑Lib): So bleibt UI reaktiv, selbst wenn Sensor‑Polling oder Messungen parallel laufen — wichtig für Boot‑Umgebung und Stabilität.

Die Web‑UI in klar getrennten Dateien (HTML/CSS/JS) halten — nicht direkt als Strings in Code einbetten. So bleiben UI und Logik sauber getrennt und wartbar.

🎯 Warum dieser Mastsensor wichtig für uns ist

Mast‑ und Sensordaten sind entscheidend für Navigation & Sicherheit — mit diesem Modul haben wir eine saubere, wartbare, leicht erweiterbare Lösung geschaffen.

Durch WLAN‑AP + WebUI ist der Sensor direkt konfigurierbar und nutzbar — smart, ohne externe Software oder komplizierten Setup.

Persistente Speicherung & modulare Firmware: Der Mastsensor lässt sich bei Bedarf erweitern — weitere Sensoren, Logging, Datenexport, Erweiterungen sind möglich.

🗺️ Mögliche Erweiterungen / To‑Dos

Logging von Mast-/Sensordaten (z. B. auf SD‑Karte oder via Web‑Download) für Analyse & Nachbereitung.

Dashboard in Web‑UI mit Diagrammen, Live‑Werten, Alarmen (z. B. bei extremer Neigung).

Schnittstelle für andere Boot‑Module: Daten über CAN/NMEA oder Web‑API bereitstellen.

Optional: OTA‑Update für Firmware + Web‑UI — erleichtert spätere Updates.

Dokumentation / Kalibrierungs-Guide + Sensordaten‑Specs (im docs‑Ordner).
