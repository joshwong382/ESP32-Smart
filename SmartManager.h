#pragma once

#include "loopable.h"
#include <ESPAsyncWebServer.h>    // ESPAsyncWebServer implementation of LinkedList

/**
 * Manages ticks/updates for various devices and sensors
 */
class SmartManager {
    public:
        SmartManager();
        ~SmartManager();

        void loopall();

        void addLoopable(Loopable* loopable);
        void addDevice(SmartDevice* dev);
        void addSensor(SmartSensorBase* dev);
    
    private:
        LinkedList<Loopable*> loopables;
        LinkedList<SmartDevice*> devices;
        LinkedList<SmartSensorBase*> sensors;
};
