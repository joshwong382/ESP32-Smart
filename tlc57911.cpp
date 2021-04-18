// From https://stackoverflow.com/questions/32675021/configuring-custom-arduino-library

#include "tlc57911.h"

TLC5971::TLC5971()
{}

void TLC5971::initializeTLC()
{
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV16);
    Commands[0] = 0x96;
    Commands[28] = 0x96;
    Commands[56] = 0x96;
    Commands[1] = 0xDF;
    Commands[29] = 0xDF;
    Commands[57] = 0xDF;
    Commands[2] = 0xFF;
    Commands[30] = 0xFF;
    Commands[58] = 0xFF;
    Commands[3] = 0xFF;
    Commands[31] = 0xFF;
    Commands[59] = 0xFF;
}

void TLC5971::setLED(int Light, uint16_t Value)
{
    LEDArray[Light] = Value;
}

void TLC5971::writeLED()
{
    for (int i=35;i>23;i--) {
        if (LEDArray[i]>0) {
            Commands[74-(2*i)] = (LEDArray[i] & 0xFF00U) >> 8U;
            Commands[75-(2*i)] = LEDArray[i] & 0xFF00U;
        }
        else {
            Commands[74-(2*i)] = 0x00;
            Commands[75-(2*i)] = 0x00;
        }
    }
    for (int i=23;i>11;i--) {
        if (LEDArray[i]>0) {
            Commands[78-(2*i)] = (LEDArray[i] & 0xFF00U) >> 8U;
            Commands[79-(2*i)] = LEDArray[i] & 0xFF00U;
        }
        else {
            Commands[78-(2*i)] = 0x00;
            Commands[79-(2*i)] = 0x00;
        }
    }
    for (int i=11;i>-1;i--) {
        if (LEDArray[i]>0) {
            Commands[82-(2*i)] = (LEDArray[i] & 0xFF00U) >> 8U;
            Commands[83-(2*i)] = LEDArray[i] & 0xFF00U;
        }
        else {
            Commands[82-(2*i)] = 0x00;
            Commands[83-(2*i)] = 0x00;
        }
    }
    for (int i=0;i<84;i++) {
        SPI.transfer(Commands[i]);
    }
}