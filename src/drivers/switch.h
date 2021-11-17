#pragma once
#include "../driver.h"

class SwitchDriver : public DeviceDriver {

    public:
    SwitchDriver(SmartDevice *_dev, const uint8_t _pin);
    void loop();

    private:
    const uint8_t pin;
    void setPower(const bool& state);
    SmartDevice* const getDev();

};