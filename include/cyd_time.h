#ifndef CYD_TIME_H
#define CYD_TIME_H

#include <time.h>
#include "credentials.h"

inline String& synchronizedStartDateTime()
{
    static String startDateTime;
    return startDateTime;
}

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

    if (!getLocalTime(&timeinfo, 10000))
        return false;

    if (synchronizedStartDateTime().length() == 0)
    {
        char buffer[32];
        if (strftime(buffer, sizeof(buffer), "%I:%M %p  %m/%d/%Y", &timeinfo) == 0)
            return false;

        synchronizedStartDateTime() = buffer;
    }

    return true;
}

inline String startDateTimeString()
{
    if (synchronizedStartDateTime().length() == 0)
        return "Time unavailable";

    return synchronizedStartDateTime();
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
