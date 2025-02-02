HomeGuardESP

HomeGuardESP este un sistem inteligent de securitate pentru locuințe, bazat pe un microcontroller ESP și comunicare prin MQTT. Proiectul permite monitorizarea senzorilor PIR și MQ2 pentru detecția mișcării și a gazelor inflamabile, iar datele sunt transmise către un broker MQTT.

Caracteristici

Comunicare prin MQTT: Datele sunt transmise în timp real către un broker MQTT.

Monitorizarea mișcării: Utilizarea unui senzor PIR pentru detecția prezenței.

Detectarea gazelor: Senzorul MQ2 detectează gaze inflamabile și fumul.

Cerințe preliminare

ESP (de exemplu, ESP8266 sau ESP32)

Senzor PIR

Senzor MQ2

VSCode cu PlatformIO

Server MQTT (de exemplu, Mosquitto)

TeraTerm pentru testare serială

Instalare și configurare

1. Configurarea mediului de dezvoltare

Instalează VSCode și extensia PlatformIO.

Clonează acest repository în VSCode.

2. Configurarea serverului MQTT

Instalează și rulează un broker MQTT, cum ar fi Mosquitto, pe un server local sau cloud.

Asigură-te că ESP-ul se conectează corect la brokerul MQTT.

3. Flash-uirea firmware-ului pe ESP

Deschide proiectul în PlatformIO.

Configurează fișierul platformio.ini pentru a include detaliile conexiunii WiFi și ale serverului MQTT.

Compilează și încarcă codul pe ESP.

4. Testarea cu TeraTerm

Conectează ESP-ul prin USB și deschide TeraTerm pentru a vizualiza mesajele seriale și a verifica funcționarea senzorilor.

Utilizare

ESP-ul citește datele de la senzorii PIR și MQ2.

Datele sunt trimise către brokerul MQTT.

În caz de detecție de mișcare sau gaz, se pot genera mesaje MQTT corespunzătoare.

Licență

Acest proiect este licențiat sub MIT License.

Pentru mai multe informații, vizitează repository-ul oficial: HomeGuardESP.
