# ESP32 Grocy Barcode Scanner
Simple ESP32 Barcode Scanner for Grocy with GM60 scanner


if you want to learn how to communicate with Grocy use V1.0.

V2.0 is much more complex and includes an MQTT interface.


## MQTT Interface

(optimized for Homeassistant with auto discovery)

![MQTT-controlls](/img/MQTT-controlls.png)

## Setup
   1. create a Grocy API-KEY
   2. Edit the Setup
   3. Wire everytihing
   4. Upload the .ino file with the Arduino IDE

## Settings
### V1.0

![settings](/img/settings.png)

### V2.0

![settings_v2](/img/settings_v2.png)

## Hardware

ESP32 Wroom https://www.amazon.de/gp/product/B08BTS62L7

GM60 Barcodescanner https://www.amazon.de/dp/B0BZC5WNF7

buzzer https://www.amazon.de/dp/B0C58DR1QV


## used Library´s

DFRobot_GM60.h https://github.com/DFRobot/DFRobot_GM60

WiFi.h

HTTPClient.h

ArduinoJson.h https://arduinojson.org/?utm_source=meta&utm_medium=library.properties

SoftwareSerial.h https://github.com/plerup/espsoftwareserial/

ArduinoMqttClient.h https://github.com/arduino-libraries/ArduinoMqttClient (only in v2.0)

## circuit diagram

![circuit diagram](/img/circuit.png)

## functions (V1.0)

   - load barcode and product data form your Grocy server
   - add Product to stock
   - consume product from stock

![functions](/img/functions.png)
