🏠 HomeGuardESP

HomeGuardESP is an intelligent home security system based on an ESP microcontroller and MQTT communication. The project allows monitoring of PIR and MQ2 sensors for motion detection and flammable gas detection, with data being transmitted to an MQTT broker.

🚀 Features

✅ MQTT Communication – Data is transmitted in real-time to an MQTT broker.
✅ Motion Monitoring – Uses a PIR sensor to detect presence.
✅ Gas Detection – The MQ2 sensor detects flammable gases and smoke.

📋 Requirements

🔹 ESP (e.g., ESP8266 or ESP32)
🔹 PIR Sensor
🔹 MQ2 Sensor
🔹 VSCode with PlatformIO
🔹 MQTT Server (e.g., Mosquitto)
🔹 TeraTerm for serial testing

🛠 Installation and Configuration

1️⃣ Setting Up the Development Environment

Install VSCode and the PlatformIO extension.

Clone this repository in VSCode.

2️⃣ Setting Up the MQTT Server

Install and run an MQTT broker, such as Mosquitto, on a local or cloud server.

Ensure the ESP connects correctly to the MQTT broker.

3️⃣ Flashing the Firmware to ESP

Open the project in PlatformIO.

Configure the platformio.ini file to include WiFi connection details and MQTT server settings.

Compile and upload the code to the ESP.

4️⃣ Testing with TeraTerm

Connect the ESP via USB and open TeraTerm.

View serial messages to check the sensor functionality.

📡 Usage

🔹 The ESP reads data from the PIR and MQ2 sensors.
🔹 Data is sent to the MQTT broker.
🔹 In case of motion or gas detection, corresponding MQTT messages can be generated.

📜 License

This project is licensed under the MIT License.

🔗 Official Repository: HomeGuardESP
