#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include "drivers/analogRGB.h"
#include "drivers/digitalRGB.h"
#include "drivers/OpenWeatherMap.h"
#include "controllers/webserver.h"
#include "controllers/homekit.h"
#include "StateManager.h"

class ArduinoSmart {

    public:
    static void loop() {
        StateManager::loopall();
    }

};