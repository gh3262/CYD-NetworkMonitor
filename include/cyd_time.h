#ifndef CYD_TIME_H
#define CYD_TIME_H

#include <time.h>
#include "credentials.h"

inline bool syncTime()
{
    configTime(
        0,
        0,
        NTP_SERVER_1,
        NTP_SERVER_2);

    setenv("TZ", TZ_STRING, 1);
    tzset();

    struct tm timeinfo;

    return getLocalTime(
        &timeinfo,
        10000);
}

inline String currentTimeString()
{
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
        return "No Time";

    char buffer[13];

    strftime(
        buffer,
        sizeof(buffer),
        "%I:%M %p",
        &timeinfo);

    return String(buffer);
}

inline String currentDateString()
{
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
        return "No Date";

    char buffer[32];

    strftime(
        buffer,
        sizeof(buffer),
        "%m/%d/%Y",
        &timeinfo);

    return String(buffer);
}
#endif
