#include "app.h"
#include <algorithm>
#include <vector>

// Scan list layout
static const int SCAN_LINE_HEIGHT = 12;
static const int SCAN_FIRST_LINE_Y = 50;
static const int SCAN_ROWS_PER_PAGE = (NAV_Y - 5 - SCAN_FIRST_LINE_Y) / SCAN_LINE_HEIGHT;

// Scan result indices sorted by signal strength, strongest first
static std::vector<int> scanOrder;

static int scanPageCount()
{
    const int count = state.networkCount < 0 ? 0 : state.networkCount;
    return (count + SCAN_ROWS_PER_PAGE - 1) / SCAN_ROWS_PER_PAGE;
}

void performScan()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar("WIFI SCAN");

    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(FONT_SMALL);

    gfx->setCursor(10, 50);
    gfx->println("Scanning...");

    state.networkCount = WiFi.scanNetworks();
    state.scanPage = 0;

    scanOrder.clear();
    for (int i = 0; i < state.networkCount; i++)
    {
        scanOrder.push_back(i);
    }

    std::sort(scanOrder.begin(), scanOrder.end(), [](int a, int b)
    {
        return WiFi.RSSI(a) > WiFi.RSSI(b);
    });
}

void drawScanScreen()
{
    clearScreen(COLOR_BACKGROUND);

    titleBar("WIFI SCAN");

    setDefaultFont();
    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(1);

    const int pageCount = scanPageCount();

    // y=25 would sit inside the 30 px title bar, so draw the count just below it
    char buffer[40];
    if (pageCount > 1)
    {
        snprintf(buffer, sizeof(buffer), "Found %d networks (page %d/%d)",
                 state.networkCount, state.scanPage + 1, pageCount);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "Found %d networks", state.networkCount < 0 ? 0 : state.networkCount);
    }
    gfx->setCursor(10, 36);
    gfx->println(buffer);

    const String connectedBssid = WiFi.BSSIDstr();
    const int firstRow = state.scanPage * SCAN_ROWS_PER_PAGE;

    for (int row = 0; row < SCAN_ROWS_PER_PAGE && firstRow + row < (int)scanOrder.size(); row++)
    {
        const int i = scanOrder[firstRow + row];

        gfx->setCursor(10, SCAN_FIRST_LINE_Y + (row * SCAN_LINE_HEIGHT));

        const bool connected = (WiFi.status() == WL_CONNECTED) && (WiFi.BSSIDstr(i) == connectedBssid);
        gfx->setTextColor(connected ? COLOR_OK : COLOR_TEXT);

        gfx->print(WiFi.SSID(i));

        gfx->print(" ");

        gfx->print(WiFi.RSSI(i));

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
    performScan();

    state.currentScreen = SCREEN_SCAN;
    drawCurrentScreen();
}

void handleScanTouch(int x, int y)
{
    // Back button
    if (navPressed1(x, y))
    {
        enterWifiStatusScreen();
        delay(300);
    }

    // Next page button (only present when there is more than one page)
    else if (scanPageCount() > 1 && navPressed2(x, y))
    {
        state.scanPage = (state.scanPage + 1) % scanPageCount();
        drawCurrentScreen();
        delay(300);
    }

    // Rescan button
    else if (navPressed3(x, y))
    {
        performScan();
        drawCurrentScreen();
        delay(300);
    }
}
