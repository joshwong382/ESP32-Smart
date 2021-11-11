#include "loopable.h"
#include "StateManager.h"

Loopable::Loopable() {
    StateManager::addLoopable(this);
}

Loopable::Loopable(const bool enable) {
    if (enable) {
      StateManager::addLoopable(this);
    }
}