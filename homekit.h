#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include <HomeSpan.h>
#include "smartdevices.h"
#include "smartsensors.h"
#include "frontend.h"

// Manage and Loop HomeSpan Instances
class HomeSpanInit : public Loopable {

  private:
    static bool homespan_init;

  public:
    static const bool getInit();

    HomeSpanInit();           // disable other instances of HomeSpanInit
    void loop();

};

class HomeSpanAccessory : public SpanAccessory {

  public:
    HomeSpanAccessory(SmartDevice* dev);
    HomeSpanAccessory(SmartSensorBase* dev);
    void init(const String name);

};

class HomeKit_RGB : public Loopable, Service::LightBulb {

  public:
    HomeKit_RGB(RGBDevice* const _internaldev);
    void loop();

  private:
    static const FrontEnd HOMEKIT_FRONTEND = FrontEnd::HomeKit;
    SpanCharacteristic *power;
    SpanCharacteristic *H;
    SpanCharacteristic *S;
    SpanCharacteristic *V;
    RGBDevice* const internalrgbdevice;

    void internal_update();   // Updated via other FrontEnds

  public:
    boolean update();         // Updated via HomeKit Frontend

};
