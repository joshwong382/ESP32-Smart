#include "homekit.h"

bool HomeSpanInit::homespan_init = false;

const bool HomeSpanInit::getInit() {
    return homespan_init;
}

HomeSpanInit::HomeSpanInit() : Loopable(!homespan_init) {     // disable other instances of HomeSpanInit
    homespan_init = true;
}

void HomeSpanInit::loop() {
    homeSpan.poll();
}


HomeSpanAccessory::HomeSpanAccessory(SmartDevice* dev) : SpanAccessory() {
    if (dev == NULL) {
        Serial.println("HomeKit initialized with nullptr");
        init("N/A");
        return;
    }

    init(dev->getName());
    
    // Add more soon
    switch (dev->type) {
        case DeviceType::BrightnessDevice:
        case DeviceType::RGBDevice:
            new HomeKit_RGB((RGBDevice*) dev);
        case DeviceType::SmartDevice:
            new HomeKit_Switch((SmartDevice*) dev);
        default:
            break;
    }
}

HomeSpanAccessory::HomeSpanAccessory(SmartSensorBase* dev) : SpanAccessory() {
    if (dev == NULL) {
        Serial.println("HomeKit initialized with nullptr");
        init("N/A");
        return;
    }

    init(dev->getName());
    
    // Add more soon
    switch (dev->type) {
        case SensorType::Weather:
            new HomeKit_Temp((Weather*) dev);
            new HomeKit_Humidity((Weather*) dev);
        default:
            break;
    }
}

void HomeSpanAccessory::init(const String name) {
    // Only init once
    if (!HomeSpanInit::getInit()) {
        new HomeSpanInit();

        // Do not use port 80
        homeSpan.setPortNum(4548);
        homeSpan.begin(Category::Lighting, "ESP32-Smart", "ESP32-Smart", "ESP32-Smart");
    }

    new Service::AccessoryInformation();
    new Characteristic::Name(name.c_str());
    new Characteristic::Manufacturer("joshua@josh-wong.net");
    new Characteristic::SerialNumber("JOSH-207");
    new Characteristic::Model("ESP32");
    new Characteristic::FirmwareRevision("2.4");
    new Characteristic::Identify();

    new Service::HAPProtocolInformation();
    new Characteristic::Version("1.2.0");
}


HomeKit_Switch::HomeKit_Switch(SmartDevice* const _dev) : Service::Switch(), dev{_dev} {
    if (_dev == NULL) {
        Serial.println("HomeKit initialized with nullptr");
        return;
    }

    power = new Characteristic::On();
}

void HomeKit_Switch::loop() {
    if (dev == NULL) return;

    const Controller update_controller = dev->statusChanged(2);
    if (update_controller == Controller::None) return;
    if (update_controller == HOMEKIT_CONTROLLER) return;

    internal_update();
}

void HomeKit_Switch::internal_update() {
    if (dev == NULL) return;
    power->setVal(dev->getPower());
}

bool HomeKit_Switch::update() {
    if (dev == NULL) return false;

    if (power->updated()) {
        dev->setPower(power->getNewVal(), HOMEKIT_CONTROLLER);
    }
    return true;
}


HomeKit_RGB::HomeKit_RGB(RGBDevice* const _internaldev) : Service::LightBulb(), internalrgbdevice{_internaldev} {
    if (_internaldev == NULL) {
        Serial.println("HomeKit initialized with nullptr");
        return;
    }

    power = new Characteristic::On();
    H = new Characteristic::Hue();
    S = new Characteristic::Saturation();
    V = new Characteristic::Brightness();
    V->setRange(1, 100, 1); // min 1%, max 100%, in 1% increments
    internal_update();
}

void HomeKit_RGB::loop() {
    if (internalrgbdevice == NULL) return;

    const Controller update_controller = internalrgbdevice->statusChanged(2);
    if (update_controller == Controller::None) return;
    if (update_controller == HOMEKIT_CONTROLLER) return;

    internal_update();
}

void HomeKit_RGB::internal_update() {
    if (internalrgbdevice == NULL) return;

    CHSV fastled_hsv = internalrgbdevice->getHSV();

    power->setVal(internalrgbdevice->getPower());
    H->setVal(fastled_hsv.hue * 360 / 255);
    S->setVal(fastled_hsv.saturation * 100 / 255);
    V->setVal(fastled_hsv.value * 100 / 255);
}

bool HomeKit_RGB::update() {
    if (internalrgbdevice == NULL) return false;

    // H[0-360]
    // S[0-100]
    // V[0-100]

    float new_h = H->getVal<float>();
    float new_s = S->getVal<float>();
    float new_v = V->getVal<float>();

    if (power->updated()) {
        internalrgbdevice->setPower(power->getNewVal(), HOMEKIT_CONTROLLER);
    }

    if (H->updated()) {
    new_h = H->getNewVal();
    }

    if (S->updated()) {
    new_s = S->getNewVal();
    }

    if (V->updated()) {
    new_v = V->getNewVal();
    }

    CRGB fastled_rgb;
    const uint8_t uint_h = uint8_t(new_h * 255 / 360 - 1);
    const uint8_t uint_s = uint8_t(new_s * 255 / 100);
    const uint8_t uint_v = uint8_t(new_v * 255 / 100);
    internalrgbdevice->setHSV(CHSV(uint_h, uint_s, uint_v), HOMEKIT_CONTROLLER);
    return true;
}


HomeKit_Temp::HomeKit_Temp(Weather* const _dev) : Service::TemperatureSensor(), dev{_dev} {
    if (_dev == NULL) {
        Serial.println("HomeKit initialized with nullptr");
        return;
    }

    temp = new Characteristic::CurrentTemperature();
}

void HomeKit_Temp::loop() {
    if (dev == NULL) return;

    static double _temp = 0;
    if (_temp == dev->getTemp()) return;

    _temp = dev->getTemp();
    internal_update();
}

void HomeKit_Temp::internal_update() {
    if (dev == NULL) return;
    double _temp = dev->getTemp();
    if (_temp < 0) _temp = 0;
    temp->setVal(_temp);
}


HomeKit_Humidity::HomeKit_Humidity(Weather* const _dev) : Service::HumiditySensor(), dev{_dev} {
    if (_dev == NULL) {
        Serial.println("HomeKit initialized with nullptr");
        return;
    }

    humid = new Characteristic::CurrentRelativeHumidity();
}

void HomeKit_Humidity::loop() {
    if (dev == NULL) return;

    static double _humid = 0;
    if (_humid == dev->getHumidity()) return;

    _humid = dev->getHumidity();
    internal_update();
}

void HomeKit_Humidity::internal_update() {
    if (dev == NULL) return;
    double _humid = dev->getHumidity();
    if (_humid < 0) _humid = 0;
    humid->setVal(_humid);
}