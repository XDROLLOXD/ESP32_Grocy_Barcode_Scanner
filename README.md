# ESP32 Grocy Barcode Scanner
Simple ESP32 Barcodescanner for Grocy with GM60 scanner

## functions
   - load barcode and product data form your Grocy server
   - add Product to stock
   - consume product from stock

![functions](/img/functions.png)


## Setup
   1. create a Grocy API-KEY
   2. Edit the Settings
   3. Wire everytihing
   4. Upload the .ino File with the Arduino IDE

## Settings

![settings](/img/settings.png)

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


## circuit diagram

![circuit diagram](/img/circuit.png)
