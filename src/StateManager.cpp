#include "StateManager.h"

void StateManager::loopall() {
    for (auto it = loopables.begin(); it != loopables.end(); ++it) {
        if ((*it) != NULL) {
            (*it)->loop();
        }
    }
}

LinkedList<Loopable*> StateManager::loopables = LinkedList<Loopable*>([](const Loopable* ptr) {
    if (ptr != NULL) {
        delete ptr;
    }
});

LinkedList<SmartDevice*> StateManager::devices = LinkedList<SmartDevice*>([](const SmartDevice* dev) {
    if (dev != NULL) {
        delete dev;
    }
});

LinkedList<SmartSensorBase*> StateManager::sensors = LinkedList<SmartSensorBase*>([](const SmartSensorBase* dev) {
    if (dev != NULL) {
        delete dev;
    }
});

void StateManager::addLoopable(Loopable* loopable) {
    loopables.add(loopable);
}

void StateManager::addDevice(SmartDevice* dev) {
    devices.add(dev);
}

void StateManager::addSensor(SmartSensorBase* dev) {
    sensors.add(dev);
}