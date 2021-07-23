#include "SmartManager.h"

void SmartManager::loopall() {
    for (auto it = loopables.begin(); it != loopables.end(); ++it) {
        if ((*it) != NULL) {
            (*it)->loop();
        }
    }
}

LinkedList<Loopable*> SmartManager::loopables = LinkedList<Loopable*>([](const Loopable* ptr) {
    if (ptr != NULL) {
        delete ptr;
    }
});

LinkedList<SmartDevice*> SmartManager::devices = LinkedList<SmartDevice*>([](const SmartDevice* dev) {
    if (dev != NULL) {
        delete dev;
    }
});

LinkedList<SmartSensorBase*> SmartManager::sensors = LinkedList<SmartSensorBase*>([](const SmartSensorBase* dev) {
    if (dev != NULL) {
        delete dev;
    }
});

void SmartManager::addLoopable(Loopable* loopable) {
    loopables.add(loopable);
}

void SmartManager::addDevice(SmartDevice* dev) {
    devices.add(dev);
}

void SmartManager::addSensor(SmartSensorBase* dev) {
    sensors.add(dev);
}