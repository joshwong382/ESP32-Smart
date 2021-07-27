#pragma once

/**
 * A type that can participate in the main loop
 */
class Loopable {
    public:
        Loopable() {};                  // Add all instances of Loopable to loopables
        virtual ~Loopable() {}          // Base classes must have a virtual deconstructor
        virtual void loop() = 0;        // Loopables must loop
};
