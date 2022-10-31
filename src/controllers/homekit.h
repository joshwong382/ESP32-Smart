#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include <HomeSpan.h>
#include "../state/smartdevices.h"
#include "../state/smartsensors.h"
#include "../controller.h"

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

class HomeKit_Switch : public Loopable, Service::Switch {

  public:
    HomeKit_Switch(SmartDevice* const _dev);
    void loop();

  private:
    static const FrontController HOMEKIT_CONTROLLER = FrontController::HomeKit;
    SpanCharacteristic *power;
    SmartDevice* const dev;
    void internal_update();

  public:
    bool update();

};

class HomeKit_RGB : public Loopable, Service::LightBulb {

  public:
    HomeKit_RGB(RGBDevice* const _internaldev);
    void loop();

  private:
    static const FrontController HOMEKIT_CONTROLLER = FrontController::HomeKit;
    SpanCharacteristic *power;
    SpanCharacteristic *H;
    SpanCharacteristic *S;
    SpanCharacteristic *V;
    RGBDevice* const internalrgbdevice;
    void internal_update();   // Updated via other Controllers

  public:
    bool update();         // Updated via HomeKit FrontController

};

class HomeKit_Temp : public Loopable, Service::TemperatureSensor {

  public:
    HomeKit_Temp(Weather* const _internaldev);
    void loop();

  private:
    static const FrontController HOMEKIT_CONTROLLER = FrontController::HomeKit;
    SpanCharacteristic *temp;
    Weather* const dev;
    void internal_update();

};

class HomeKit_Humidity : public Loopable, Service::HumiditySensor {

  public:
    HomeKit_Humidity(Weather* const _internaldev);
    void loop();

  private:
    static const FrontController HOMEKIT_CONTROLLER = FrontController::HomeKit;
    SpanCharacteristic *humid;
    Weather* const dev;
    void internal_update();

};