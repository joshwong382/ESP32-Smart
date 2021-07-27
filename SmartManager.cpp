#include "SmartManager.h"

SmartManager::SmartManager() :
    loopables{LinkedList<Loopable*>([](const Loopable* ptr) {
        if (ptr != NULL) {
            delete ptr;
        }
    })},
    devices{LinkedList<SmartDevice*>([](const SmartDevice* dev) {
        if (dev != NULL) {
            delete dev;
        }
    })},
    sensors{LinkedList<SmartSensorBase*>([](const SmartSensorBase* dev) {
        if (dev != NULL) {
            delete dev;
        }
    })}
{}

SmartManager::~SmartManager() {}

void SmartManager::loopall() {
    for (auto it = this->loopables.begin(); it != this->loopables.end(); ++it) {
        if ((*it) != NULL) {
            (*it)->loop();
        }
    }
}

void SmartManager::addLoopable(Loopable* loopable) {
    this->loopables.add(loopable);
}

void SmartManager::addDevice(SmartDevice* dev) {
    this->devices.add(dev);
}

void SmartManager::addSensor(SmartSensorBase* dev) {
    this->sensors.add(dev);
}
