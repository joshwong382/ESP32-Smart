#include "loopable.h"
#include "SmartManager.h"

Loopable::Loopable() {
    SmartManager::addLoopable(this);
}

Loopable::Loopable(const bool enable) {
    if (enable) {
      SmartManager::addLoopable(this);
    }
}