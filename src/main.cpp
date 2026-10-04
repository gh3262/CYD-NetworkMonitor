#include <Arduino.h>
#include "cyd.h"

void setup()
{
    Serial.begin(115200);

    if (!initCYD())
    {
        while (true)
        {
        }
    }

    showSplash("Network Monitor");
}

void loop()
{
}