#include "app.h"
#include <algorithm>
#include <vector>

// Scan list layout
static const int SCAN_LINE_HEIGHT = 12;
static const int SCAN_FIRST_LINE_Y = 50;
static const int SCAN_ROWS_PER_PAGE = (NAV_Y - 5 - SCAN_FIRST_LINE_Y) / SCAN_LINE_HEIGHT;

// Results copied out of the WiFi driver so its scan memory can be freed at once,
// sorted by signal strength, strongest first
struct ScanEntry
{
    char ssid[33];
    int rssi;
    bool connected;
};

static std::vector<ScanEntry> scanList;

static int scanPageCount()
{
    const int count = state.networkCount < 0 ? 0 : state.networkCount;
    return (count + SCAN_ROWS_PER_PAGE - 1) / SCAN_ROWS_PER_PAGE;
}

static const unsigned long SCAN_TIMEOUT_MS = 15000;

static bool scanning = false;
static unsigned long scanStartMs = 0;

bool scanInProgress()
{
    return scanning;
}

// Starts an asynchronous scan; updateScan() collects the results
static void startScan()
{
    if (scanning)
        return;

    // A pending connection attempt keeps the radio busy and makes the scan
    // fail, so drop it first; maintainWiFi() restarts it later
    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.disconnect();
        delay(100);
    }

    scanList.clear();
    state.networkCount = 0;
    state.scanPage = 0;

    // The core aborts a scan after 20 x max_ms_per_chan (6s by default), which
    // is too short while connected, so allow 12s
    const int started = WiFi.scanNetworks(true, false, false, 600);
    scanning = (started == WIFI_SCAN_RUNNING);
    Serial.printf("Scan start: %d, status %d\n", started, (int)WiFi.status());
    scanStartMs = millis();
}

// Called from loop(): picks up the finished scan without blocking
void updateScan()
{
    if (!scanning)
        return;

    const int found = WiFi.scanComplete();

    if (found == WIFI_SCAN_RUNNING && millis() - scanStartMs < SCAN_TIMEOUT_MS)
        return;

    Serial.printf("Scan done: %d after %lu ms\n", found, millis() - scanStartMs);

    scanning = false;
    scanList.clear();

    const bool online = (WiFi.status() == WL_CONNECTED);
    uint8_t connectedBssid[6] = {0};
    if (online)
        memcpy(connectedBssid, WiFi.BSSID(), 6);

    for (int i = 0; i < found; i++)
    {
        ScanEntry entry;
        strlcpy(entry.ssid, WiFi.SSID(i).c_str(), sizeof(entry.ssid));
        entry.rssi = WiFi.RSSI(i);
        entry.connected = online && memcmp(WiFi.BSSID(i), connectedBssid, 6) == 0;
        scanList.push_back(entry);
    }

    // Results are copied, so release the driver's copy now (also on failure)
    WiFi.scanDelete();

    std::sort(scanList.begin(), scanList.end(), [](const ScanEntry &a, const ScanEntry &b)
    {
        return a.rssi > b.rssi;
    });

    state.networkCount = (int)scanList.size();

    if (state.currentScreen == SCREEN_SCAN)
        drawCurrentScreen();
}
void drawScanScreen()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar("WIFI SCAN");

    setDefaultFont();
    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(1);

    if (scanning)
    {
        gfx->setCursor(10, 36);
        gfx->println("Scanning...");

        drawNav1("Back");
        drawNav4("Home");
        return;
    }

    const int pageCount = scanPageCount();

    // y=25 would sit inside the 30 px title bar, so draw the count just below it
    char buffer[40];
    if (pageCount > 1)
    {
        snprintf(buffer, sizeof(buffer), "%d networks (page %d/%d)",
                 state.networkCount, state.scanPage + 1, pageCount);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "%d networks", state.networkCount < 0 ? 0 : state.networkCount);
    }
    gfx->setCursor(10, 36);
    gfx->setTextColor(COLOR_LABEL_TEXT);
    gfx->print("Found ");
    gfx->setTextColor(COLOR_TEXT);
    gfx->println(buffer);

    const int firstRow = state.scanPage * SCAN_ROWS_PER_PAGE;

    for (int row = 0; row < SCAN_ROWS_PER_PAGE && firstRow + row < (int)scanList.size(); row++)
    {
        const ScanEntry &entry = scanList[firstRow + row];

        gfx->setCursor(10, SCAN_FIRST_LINE_Y + (row * SCAN_LINE_HEIGHT));
        gfx->setTextColor(entry.connected ? COLOR_OK : COLOR_TEXT);

        gfx->print(entry.ssid);
        gfx->print(" ");
        gfx->print(entry.rssi);
        gfx->println(" dBm");
    }
    setDefaultFont();

    drawNav1("Back");

    if (pageCount > 1)
    {
        drawNav2("Next");
    }

    drawNav3("Rescan");
    drawNav4("Home");
}

void enterScanScreen()
{
    startScan();

    state.currentScreen = SCREEN_SCAN;
    drawCurrentScreen();
}

void handleScanTouch(int x, int y)
{
    // Back button
    if (navPressed1(x, y))
    {
        enterWifiStatusScreen();
    }

    // Next page button (only present when there is more than one page)
    else if (scanPageCount() > 1 && navPressed2(x, y))
    {
        state.scanPage = (state.scanPage + 1) % scanPageCount();
        drawCurrentScreen();
    }

    // Rescan button
    else if (navPressed3(x, y))
    {
        startScan();
        drawCurrentScreen();
    }
}
