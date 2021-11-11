# ArduinoSmart
This ESP32 ArduinoSmart Library allows multi-platform control of different types of IoT sensors/devices.

## Setup
Connect to WiFi using the HomeSpan CLI. More details: [Click Here](https://github.com/HomeSpan/HomeSpan/blob/master/docs/CLI.md).

## Features
Controller Protocols:
- HomeKit
- HTTP

Types of Devices:
- On/Off Switch
- Brightness Light
- RGB Light

Types of Sensors:
- Temp & Humidity

Light Drivers:
- [FastLED](http://fastled.io/) (Individually Addressable)
- Analog (PWM)

Special Features:
- Flash to music via trigger PIN
- RGB Rainbow mode

## How to use
TBA :)

## Newest Changes
- Refactor HomeKit
- Refactor Analog and DigitalRGB
- Add Loopable Class

## Future Changes
- Rewrite Controller base class
- Change how device capabilities are defined
- Use GET variables exclusively for HTTP_API
- Add Alexa control
