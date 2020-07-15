#pragma once
#include <WString.h>
#include <IPAddress.h>
#include <FastLED.h>

String ip2String(const IPAddress& ipAddress);
bool isValidIP(const IPAddress& ipAddress);
bool wifi_check(char* ssid, char* pass);

void deskLED_callback(uint8_t brightness, uint32_t rgb);
void musicLED_callback(uint8_t brightness);

String colorinttohexstr(uint8_t color);
void setAnalogRGB(uint32_t rgb, uint8_t brightness, uint8_t REDPIN, uint8_t GREENPIN, uint8_t BLUEPIN);
void setAnalogRGB(uint8_t red, uint8_t blue, uint8_t green, uint8_t brightness, uint8_t REDPIN, uint8_t GREENPIN, uint8_t BLUEPIN);
void setAnalogRGB(const CRGB& rgb, uint8_t brightness, uint8_t REDPIN, uint8_t GREENPIN, uint8_t BLUEPIN);
uint8_t rgbtopercent(uint8_t rgb);
uint8_t percenttorgb(uint8_t percent);