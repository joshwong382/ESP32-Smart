#include "loopable.h"

Loopable::Loopable() {
    loopables.add(this);
}

void Loopable::loopall() {
    for (auto it = loopables.begin(); it != loopables.end(); ++it) {
        if ((*it) != NULL) {
            (*it)->loop();
        }
    }
}

LinkedList<Loopable*> Loopable::loopables = LinkedList<Loopable*>([](const Loopable* ptr) {
    if (ptr != NULL) {
        delete ptr;
    }
});