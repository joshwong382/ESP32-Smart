# ESP32-Lighting
This ESP32 Arduino program allows multi-platform control of different types of IoT sensors/devices.

## Setup
Connect to WiFi using HomeSpan. More details: [Click Here](https://github.com/HomeSpan/HomeSpan).

## Features
Control Protocols:
- HomeKit
- HTTP

Types of Devices:
- On/Off Switch
- Brightness Light
- RGB Light

Light Drivers:
- [FastLED](http://fastled.io/) (Individually Addressable)
- Analog (PWM)

Special Features:
- Flash to music via trigger PIN
- RGB Rainbow mode

## Newest Changes
- Rewrite Analog RGB to OOP
- Rewrite WebServer to a class
- Added OpenWeatherMap for outdoor weather

## Future Changes
- Rewrite Digital RGB to OOP
- Rewrite HomeKit to OOP
- Rewrite FrontEnd base class
- Rewrite Backends to classes
- Change how device capabilities are defined
- Use GET variables exclusively for HTTP_API
- Add Alexa control
