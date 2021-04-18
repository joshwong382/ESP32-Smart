// From https://stackoverflow.com/questions/32675021/configuring-custom-arduino-library

#ifndef TLC5971_h
#define TLC5971_h
#include <Arduino.h>
#include <SPI.h>

class TLC5971
{
public:
    TLC5971();
    void initializeTLC();
    void setLED(int Light, uint16_t Value);
    void writeLED();
private:
    uint16_t LEDArray[36];
    byte Commands[84];
};

#endif /* TLC5971_h */