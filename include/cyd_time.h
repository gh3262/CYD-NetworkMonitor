#ifndef CYD_TIME_H
#define CYD_TIME_H

#include <time.h>
#include "esp_sntp.h"
#include "credentials.h"

// Start time text; empty until the first successful sync
inline char* synchronizedStartDateTime()
{
    static char startDateTime[32] = "";
    return startDateTime;
}

// Set by the SNTP callback each time a real NTP reply updates the clock
inline volatile bool& ntpReplyReceived()
{
    static volatile bool received = false;
    return received;
}

inline void onNtpSync(struct timeval *)
{
    ntpReplyReceived() = true;
}

// Starts an NTP request and returns immediately; ntpReplyReceived() turns
// true when the reply arrives, then call finishTimeSync()
inline void startTimeSync()
{
    ntpReplyReceived() = false;
    sntp_set_time_sync_notification_cb(onNtpSync);

    configTime(
        0,
        0,
        NTP_SERVER_1,
        NTP_SERVER_2);

    setenv("TZ", TZ_STRING, 1);
    tzset();
}

// Call once ntpReplyReceived() is true
inline bool finishTimeSync()
{
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo, 0))
        return false;

    if (synchronizedStartDateTime()[0] == '\0')
    {
        char buffer[32];
        if (strftime(buffer, sizeof(buffer), "%I:%M %p  %m/%d/%Y", &timeinfo) == 0)
            return false;

        strcpy(synchronizedStartDateTime(), buffer);
    }

    return true;
}

// Blocking version, for startup only
inline bool syncTime()
{
    startTimeSync();

    // getLocalTime() alone returns at once if the clock was already set, so
    // wait for an actual NTP reply to know this sync really happened
    const unsigned long start = millis();
    while (!ntpReplyReceived())
    {
        if (millis() - start > 10000)
            return false;

        delay(50);
    }

    return finishTimeSync();
}

inline const char* startDateTimeString()
{
    if (synchronizedStartDateTime()[0] == '\0')
        return "Time unavailable";

    return synchronizedStartDateTime();
}

// The returned pointers are to static buffers: use them right away
inline const char* currentTimeString()
{
    static char buffer[13];
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo, 0))
        return "No Time";

    strftime(buffer, sizeof(buffer), "%I:%M %p", &timeinfo);
    return buffer;
}

inline const char* currentDateString()
{
    static char buffer[32];
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo, 0))
        return "No Date";

    strftime(buffer, sizeof(buffer), "%m/%d/%Y", &timeinfo);
    return buffer;
}
#endif