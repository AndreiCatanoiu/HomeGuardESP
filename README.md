# HomeGuardESP

**HomeGuardESP** is an intelligent home security system developed on the ESP8266/ESP32 platform, designed to monitor and protect your home using sensors and Wi-Fi communication. The project allows you to detect critical events (e.g., unauthorized entries or emergency situations) and receive real-time alerts, with the ability to control and configure the system remotely via a mobile application. 🏡🔒

---

## Description

The project was designed to offer an affordable and easy-to-implement security solution for homes. By integrating various sensor types (motion, contact, smoke, etc.), the system ensures complete monitoring of your home environment, notifying the user in case of any suspicious or critical events. 🚨📡

> **Note:** This implementation includes MQTT integration and Wi-Fi connectivity, along with support for an MQ2 gas sensor and a PIR motion sensor. Additionally, a mobile application is planned to receive MQTT messages and display real-time sensor statuses, with more features to be added in the future.

---

## Features

- **Real-time monitoring:** Instantly detects security events and triggers alarms. ⏱️🔍
- **Customizable alerts:** Configure alerts and notifications according to your preferences. 🛎️
- **Easy installation:** Step-by-step guide for setting up and installing the system. 🔧👨‍🔧
- **MQTT Integration:** Seamless communication between devices, enabling real-time updates to a mobile app. 📲

---

## Required Hardware

- **ESP8266** or **ESP32** board
- Sensors (e.g., **PIR** for motion, **magnetic contacts**, **smoke detectors**, etc.)
- Integrated Wi-Fi module

---

## Required Software

- [Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO](https://platformio.org/)
- Support for the ESP board (follow the instructions in the [ESP8266 documentation](https://github.com/esp8266/Arduino) or the [ESP32 documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html))

---

## Installation and Configuration

1. **Clone the repository:**

    ```bash
    git clone https://github.com/AndreiCatanoiu/HomeGuardESP.git
    ```

2. **Configure the development environment:**
   - Install [Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO](https://platformio.org/).
   - Add support for **ESP8266/ESP32** as per the official documentation.

3. **Customize the configuration:**
   - Open the configuration file (e.g., `config.h` or similar) and update the Wi-Fi connection details, as well as any other specific settings (e.g., sensor thresholds, MQTT broker settings, notification addresses, etc.).

4. **Upload the code to the ESP board:**
   - Connect the board to your computer.
   - Select the correct port and board in Arduino IDE/PlatformIO.
   - Compile and upload the program to the ESP board.

---

## Usage

After configuration, the system will begin continuous monitoring of your home. You can access the mobile application (if implemented) to:
- View the real-time status of sensors.
- Configure alarms and notifications.
- Review the alert history.
- Receive MQTT messages that provide live updates on the sensor data.

---

## Contributions

Contributions are welcome! If you have suggestions, improvements, or have found any bugs, please open an [issue](https://github.com/AndreiCatanoiu/HomeGuardESP/issues) or propose a [pull request](https://github.com/AndreiCatanoiu/HomeGuardESP/pulls). 🤝

---

## License

This project is distributed under the [MIT License](https://opensource.org/licenses/MIT). Full details can be found in the [LICENSE](./LICENSE) file. 📜

---

## Contact

For questions, suggestions, or collaborations, you can reach me at:
- **GitHub:** [AndreiCatanoiu](https://github.com/AndreiCatanoiu)

---
