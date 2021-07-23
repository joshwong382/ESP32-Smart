#pragma once
#include "loopable.h"
#include "smartdevices.h"
#include "smartsensors.h"

class SmartManager {
    public:
        static void loopall();

    private:
        // Contributers
        friend class Loopable;
        friend class SmartDevice;
        friend class SmartSensorBase;

        // Consumers
        friend class WebServer;

        static LinkedList<Loopable*> loopables;
        static LinkedList<SmartDevice*> devices;
        static LinkedList<SmartSensorBase*> sensors;

        static void addLoopable(Loopable* loopable);
        static void addDevice(SmartDevice* dev);
        static void addSensor(SmartSensorBase* dev);
};