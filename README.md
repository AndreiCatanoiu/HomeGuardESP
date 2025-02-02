# HomeGuardESP

**HomeGuardESP** este un sistem de securitate inteligent pentru locuință, dezvoltat pe platforma ESP8266/ESP32, care monitorizează și protejează casa prin utilizarea senzorilor și a comunicației Wi-Fi. Proiectul îți permite să detectezi evenimentele critice (de exemplu, intrări neautorizate sau situații de urgență) și să primești alerte în timp real, având posibilitatea de a controla și configura sistemul de la distanță printr-o interfață web.

---

## Descriere

Proiectul a fost conceput pentru a oferi o soluție de securitate accesibilă și ușor de implementat pentru locuințe. Prin integrarea diverselor tipuri de senzori (mișcare, contact, fum etc.), sistemul asigură o monitorizare completă a mediului casnic, notificând utilizatorul în cazul oricăror evenimente suspecte sau critice.

---

## Caracteristici

- **Monitorizare în timp real:** Detectează imediat evenimentele de securitate și declanșează alarme.
- **Alarme personalizabile:** Configurarea alertelor și notificărilor conform preferințelor utilizatorului.
- **Interfață web:** Acces remote pentru vizualizarea statusului sistemului și configurarea parametrilor.
- **Suport pentru diverse senzori:** Compatibil cu senzori de mișcare, senzori de contact (reed switches), detectoare de fum și multe altele.
- **Ușor de instalat:** Ghid pas cu pas pentru configurarea și instalarea sistemului.

---

## Hardware necesar

- Placă **ESP8266** sau **ESP32**
- Senzori (ex.: **PIR** pentru mișcare, **contacte magnetice**, **detectoare de fum** etc.)
- Modul Wi-Fi integrat
- Alte componente electronice (rezistențe, LED-uri, buzzer etc.)

---

## Software necesar

- [Arduino IDE](https://www.arduino.cc/en/software) sau [PlatformIO](https://platformio.org/)
- Suportul pentru placa ESP (urmează instrucțiunile din [documentația ESP8266](https://github.com/esp8266/Arduino) sau [documentația ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html))
- Alte librării specificate în cod (consultați eventualul fișier `libraries.txt` sau documentația inline)

---

## Instalare și configurare

1. **Clonarea repository-ului:**

    ```bash
    git clone https://github.com/AndreiCatanoiu/HomeGuardESP.git
    ```

2. **Configurarea mediului de dezvoltare:**
   - Instalează [Arduino IDE](https://www.arduino.cc/en/software) sau [PlatformIO](https://platformio.org/).
   - Adaugă suportul pentru **ESP8266/ESP32** conform documentației oficiale.

3. **Personalizarea configurației:**
   - Deschide fișierul de configurare (de exemplu, `config.h` sau similar) și actualizează datele de conectare la rețeaua Wi-Fi, precum și orice alte setări specifice (ex.: praguri pentru senzori, adrese pentru notificări etc.).

4. **Încărcarea codului pe placa ESP:**
   - Conectează placa la calculator.
   - Selectează portul și modelul corect în Arduino IDE/PlatformIO.
   - Compilează și încarcă programul pe placa ESP.

---

## Utilizare

După configurare, sistemul va începe monitorizarea continuă a locuinței. Poți accesa interfața web (dacă a fost implementată) pentru a:
- Vizualiza statusul în timp real al senzorilor.
- Configura alarme și notificări.
- Consulta istoricul alertelor.

---

## Contribuții

Contribuțiile sunt binevenite! Dacă ai sugestii, îmbunătățiri sau ai identificat bug-uri, te rog să deschizi un [issue](https://github.com/AndreiCatanoiu/HomeGuardESP/issues) sau să propui un [pull request](https://github.com/AndreiCatanoiu/HomeGuardESP/pulls).

---

## Licență

Acest proiect este distribuit sub licența [MIT License](https://opensource.org/licenses/MIT). Detalii complete se regăsesc în fișierul [LICENSE](./LICENSE).

---

## Contact

Pentru întrebări, sugestii sau colaborări, mă poți contacta la:
- **Email:** andrei.catanoiu@gmail.com
- **GitHub:** [AndreiCatanoiu](https://github.com/AndreiCatanoiu)

---

