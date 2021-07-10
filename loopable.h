#pragma once
#include <ESPAsyncWebServer.h>      // ESPAsyncWebServer implementation of LinkedList

class Loopable {
    public:
        Loopable();                 // Add all instances of Loopable to loopables
        virtual void loop() = 0;    // Loopables must loop

        static void loopall();      // Loop all loopables

    private:
        static LinkedList<Loopable*> loopables;
};
