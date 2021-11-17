#include <Arduino.h>
#include "switch.h"
    
SwitchDriver::SwitchDriver(SmartDevice *_dev, const uint8_t _pin) : DeviceDriver(_dev), pin{_pin} {
    pinMode(pin, OUTPUT);
}

void SwitchDriver::loop() {
    const bool update = static_cast<bool>(getDev()->statusChanged(0));

    if (update) {
        setPower(getDev()->getPower());
    }
}

void SwitchDriver::setPower(const bool& state) {
    digitalWrite(pin, state);
}

SmartDevice* const SwitchDriver::getDev() {
    return dev;
}
