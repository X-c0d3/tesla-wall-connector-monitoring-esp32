#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <SocketIoClient.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include <WiFiClientSecure.h>
#include <arduino-timer.h>

#include "Models/teslaApiReponse.h"
#include "buttonAction.h"
#include "makeResponse.h"
#include "settings.h"
#include "teslaMate.h"
#include "utility.h"
#include "wifiMan.h"

TFT_eSPI tft = TFT_eSPI();
SocketIoClient webSocket;
TeslaApiReponse teslaWallConnect;
TeslaMateResponse teslaMate;

auto timer = timer_create_default();  // create a timer with default settings
// Max values for bar graph scaling
const float MAX_CURRENT = 32.0;
const float MAX_VOLTAGE = 240.0;
const float MAX_SESSION_ENERGY = 40000.0;  // Wh
const float MAX_CHARGE_POWER = 7500.0;     // Wh
const float MAX_LIFETIME_ENERGY = 5000.0;  // kWh
const int ESP32C3_LED = 8;
bool isStarted = false;

enum Button {
    BTN_NONE,
    BTN_START,
    BTN_EXIT
};

void drawHeader() {
    tft.fillRect(0, 0, 480, 30, TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("Tesla Wall Connector", 10, 5);
    tft.setTextSize(1.9);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("Firmware: " + String(teslaWallConnect.firmware_version), 280, 5);
    tft.drawString("Serial: " + String(teslaWallConnect.serial_number), 280, 18);
    tft.drawLine(0, 30, 480, 30, TFT_GREEN);
}

void drawVitals() {
    int y = 40;
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    drawStatus(tft, 107, y + 6, teslaWallConnect.vehicle_connected);
    tft.drawString("Vehicle", 120, y);
    drawStatus(tft, 247, y + 6, teslaWallConnect.contactor_closed);
    tft.drawString("Relay Contactor", 260, y);
    y += 25;

    tft.setTextSize(1.9);
    tft.drawString("Grid AC: " + String(teslaWallConnect.grid_v) + " V", 10, y);
    drawBar(tft, 120, y, 100, 10, teslaWallConnect.grid_v, MAX_VOLTAGE, TFT_BLUE);

    tft.drawString("Vehicle Current: " + String(teslaWallConnect.vehicle_current_a) + " A", 230, y);
    drawBar(tft, 375, y, 100, 10, teslaWallConnect.vehicle_current_a, MAX_CURRENT, TFT_BLUE);
    y += 15;

    // A
    tft.drawString("CurrentA: " + String(teslaWallConnect.currentA_a) + " A", 10, y);
    drawBar(tft, 120, y, 100, 10, teslaWallConnect.currentA_a, MAX_CURRENT, TFT_YELLOW);

    tft.drawString("VoltageA: " + String(teslaWallConnect.voltageA_v) + " V", 250, y);
    drawBar(tft, 375, y, 100, 10, teslaWallConnect.voltageA_v, MAX_VOLTAGE, TFT_YELLOW);
    y += 15;

    // B
    tft.drawString("CurrentB: " + String(teslaWallConnect.currentB_a) + " A", 10, y);
    drawBar(tft, 120, y, 100, 10, teslaWallConnect.currentB_a, MAX_CURRENT, TFT_YELLOW);

    tft.drawString("VoltageB: " + String(teslaWallConnect.voltageB_v) + " V", 250, y);
    drawBar(tft, 375, y, 100, 10, teslaWallConnect.voltageB_v, MAX_VOLTAGE, TFT_YELLOW);
    y += 15;

    // C
    tft.drawString("CurrentC: " + String(teslaWallConnect.currentC_a) + " A", 10, y);
    drawBar(tft, 120, y, 100, 10, teslaWallConnect.currentC_a, MAX_CURRENT, TFT_YELLOW);

    tft.drawString("VoltageC: " + String(teslaWallConnect.voltageC_v) + " V", 250, y);
    drawBar(tft, 375, y, 100, 10, teslaWallConnect.voltageC_v, MAX_VOLTAGE, TFT_YELLOW);
    y += 15;

    // N
    tft.drawString("CurrentN: " + String(teslaWallConnect.currentN_a) + " A", 10, y);
    drawBar(tft, 120, y, 100, 10, teslaWallConnect.currentN_a, MAX_CURRENT, TFT_YELLOW);
    y += 15;

    tft.drawString("Frequency: " + String(teslaWallConnect.grid_hz) + " Hz", 10, y);
    tft.drawString("Session: " + smartDuration(teslaWallConnect.session_s), 220, y);
    y += 15;

    float chargerPower = (teslaWallConnect.grid_v * teslaWallConnect.vehicle_current_a);
    drawFixedText(tft, 10, y, 150, 10, "Charger Power: " + smartEnergyW(chargerPower), TFT_WHITE, TFT_BLACK, 1.9);
    drawBar(tft, 200, y, 150, 10, chargerPower, MAX_CHARGE_POWER, TFT_GREENYELLOW);
    y += 15;

    tft.drawString("Session Energy: " + smartEnergyW(teslaWallConnect.session_energy_wh), 10, y);
    drawBar(tft, 200, y, 150, 10, teslaWallConnect.session_energy_wh, MAX_SESSION_ENERGY, TFT_GREENYELLOW);
    tft.drawString("Exit", 420, y);
    y += 15;

    tft.drawString("PCBA Temp: " + String(teslaWallConnect.pcba_temp_c) + " C", 10, y);
    tft.drawString("Handle Temp: " + String(teslaWallConnect.handle_temp_c) + " C", 220, y);
    y += 15;

    tft.drawString("Pilot High: " + String(teslaWallConnect.pilot_high_v) + " V", 10, y);
    tft.drawString("Pilot Low: " + String(teslaWallConnect.pilot_low_v) + " V", 220, y);
    y += 15;

    String chargeStatus;
    switch (teslaWallConnect.evse_state) {
        case 1:
            chargeStatus = "Not Connected";
            break;
        case 4:
            chargeStatus = "Finished";
            break;
        case 9:
            chargeStatus = "Charging Stopped";
            break;
        case 11:
            chargeStatus = "Charging";
            break;
        default:
            chargeStatus = "Unknown";
    }

    drawFixedText(tft, 10, y, 200, 10, "State: " + chargeStatus, TFT_WHITE, TFT_BLACK, 1.9);
    tft.drawString("Prox: " + String(teslaWallConnect.prox_v) + " V", 220, y);
    y += 15;

    tft.drawString("Input thermopile: " + String(teslaWallConnect.input_thermopile_uv, 0), 10, y);
    tft.drawString("Mcu temp: " + String(teslaWallConnect.mcu_temp_c) + " C", 220, y);
    y += 15;
    tft.drawString("Relay coil: " + String(teslaWallConnect.relay_coil_v) + " V", 10, y);
    y += 15;
    tft.drawString("Range (rated): " + String(teslaMate.range_rated) + " Km", 220, y);
}

void drawLifetime() {
    float uptimeDays = teslaWallConnect.uptime_l_s / 86400;

    int y = 260;
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(1.9);

    tft.drawString("Contactor cycles: " + String(teslaWallConnect.contactor_cycles), 10, y);
    y += 15;
    tft.drawString("Energy: " + smartEnergyW(teslaWallConnect.energy_wh), 10, y);
    drawBar(tft, 200, y, 150, 10, teslaWallConnect.energy_wh / 1000.0, MAX_LIFETIME_ENERGY, TFT_ORANGE);
    y += 15;
    drawFixedText(tft, 10, y, 200, 10, "Uptime: " + String(uptimeDays, 0) + " days", TFT_YELLOW, TFT_BLACK, 1.9);
}

void drawWifi() {
    int y = 305;
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.setTextSize(1.9);
    tft.drawString("SSID: " + String(teslaWallConnect.wifi_ssid), 10, y);
    tft.drawString("RSSI: " + String(teslaWallConnect.wifi_rssi) + " dBm", 130, y);
    tft.drawString("IP: " + String(teslaWallConnect.wifi_infra_ip), 260, y);

    drawStatus(tft, 410, y + 3, teslaWallConnect.internet);
    tft.drawString("Internet", 420, y);
}

void drawTeslaMate() {
    int y = 215;
    if (teslaMate.soc > 0) {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setTextSize(3);
        tft.drawString(String(teslaMate.soc) + "%", 410, y);
        y += 45;
    }

    if (teslaMate.mileage > 0) {
        tft.setTextSize(1.9);
        tft.drawString(String(teslaMate.mileage) + " km", 410, y);
    }
}

bool ledState = false;
bool updateDisplayTeslaWallConnect(void*) {
    ledState = !ledState;
    digitalWrite(ESP32C3_LED, ledState);

    if (isStarted) {
        // Version
        String versionResp = httpGet(String(TESLA_WALLCONNECT_API) + "version");
        StaticJsonDocument<512> verDoc;
        deserializeJson(verDoc, versionResp);

        // wifi_status
        String wifiResp = httpGet(String(TESLA_WALLCONNECT_API) + "wifi_status");
        StaticJsonDocument<512> wifiDoc;
        deserializeJson(wifiDoc, wifiResp);

        // Vitals
        String vitalsResp = httpGet(String(TESLA_WALLCONNECT_API) + "vitals");
        StaticJsonDocument<2048> vitalsDoc;
        deserializeJson(vitalsDoc, vitalsResp);

        // Lifetime
        String lifetimeResp = httpGet(String(TESLA_WALLCONNECT_API) + "lifetime");
        StaticJsonDocument<1024> lifetimeDoc;
        deserializeJson(lifetimeDoc, lifetimeResp);

        convertFromJson(verDoc.as<JsonVariantConst>(),
                        wifiDoc.as<JsonVariantConst>(),
                        vitalsDoc.as<JsonVariantConst>(),
                        lifetimeDoc.as<JsonVariantConst>(),
                        teslaWallConnect);

        drawHeader();
        drawVitals();
        drawLifetime();
        drawWifi();
        drawTeslaMate();
    }

    return true;
}

bool updateDisplayTeslaMate(void*) {
    // TeslaMate
    String teslaMateResp = httpGet(TESLAMATE_URL);
    if (teslaMateResp.length() > 1000) {
        teslaMate = convertToTeslaMate(teslaMateResp);
        printTeslaMateResults();
    }

    if (!isStarted) {
        int y = 60;
        tft.setTextColor(TFT_BLACK, TFT_WHITE);
        tft.setTextSize(1.9);
        tft.drawString("Status: " + String(teslaMate.status), 10, y);
        y += 15;
        tft.drawString("Range (rated): " + String(teslaMate.range_rated) + " km", 10, y);
        y += 15;

        if (teslaMate.charge_limit > 0) {
            tft.drawString("Charge Limit: " + String(teslaMate.charge_limit) + " km", 10, y);
            y += 15;
        }

        if (teslaMate.temp_inside > 0) {
            tft.drawString("Inside Temp: " + String(teslaMate.temp_inside) + " C", 10, y);
            y += 15;
        }
        tft.drawString("Mileage: " + String(teslaMate.mileage) + " km", 10, y);

        tft.setTextSize(4);
        tft.setTextColor(TFT_RED, TFT_WHITE);
        tft.drawString(String(teslaMate.soc) + "%", 400, 60);

        tft.setTextSize(1.9);
        tft.setTextColor(TFT_BLACK, TFT_WHITE);
        tft.drawString(String(WiFi.localIP().toString()), 387, 285);
        tft.drawString("v " + String(teslaMate.version), 410, 300);
    }
    // createResponse(webSocket, teslaWallConnect);
    return true;  // repeat? true
}

void event(const char* payload, size_t length) {
    // Serial.println("Socket.io Event Received:");
    // Serial.println(String(payload).substring(0, length));
}

void gradientButton(int x, int y, int w, int h) {
    for (int i = 0; i < h; i++) {
        uint8_t c = map(i, 0, h, 20, 100);
        uint16_t color = tft.color565(c, 0, 0);
        tft.drawFastHLine(x, y + i, w, color);
    }

    tft.drawRoundRect(x, y, w, h, 10, TFT_WHITE);
    tft.drawString("START", x + 20, y + 15);
}

void createStartButton() {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(2);
    tft.fillRoundRect(140, 260, 200, 50, 10, TFT_BLACK);
    tft.drawRoundRect(140, 260, 200, 50, 10, TFT_WHITE);
    tft.drawCentreString("START", 240, 270, 2);
}

// callback for drwaing JPG into TFT_eSPI
bool jpgRender(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    tft.pushImage(x, y, w, h, bitmap);
    return true;
}

void initialMainPage() {
    TJpgDec.setCallback(jpgRender);

    String mainLogo = "/mainLogo.jpg";
    File file = SPIFFS.open(mainLogo);
    if (!file) {
        Serial.println("Failed to open " + mainLogo);
        return;
    }
    // Load Image
    TJpgDec.drawFsJpg(0, 0, mainLogo);

    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("Tesla Model Y/LR (Juniper)", 90, 10);
    tft.drawString("LOADING...", 190, 290);
}

void loadBootLogo() {
    TJpgDec.setCallback(jpgRender);

    String boolLogo = "/tesla-boot-logo.jpg";
    File file = SPIFFS.open(boolLogo);
    if (!file) {
        Serial.println("Failed to open " + boolLogo);
        return;
    }
    // Load Image
    TJpgDec.drawFsJpg(0, 0, boolLogo);
}

void addCarLogo() {
    String teslaLogo2 = "/tesla_logo2.jpg";  // load image from SPIFFS
    File file = SPIFFS.open(teslaLogo2);

    tft.fillScreen(TFT_BLACK);
    tft.setSwapBytes(true);

    TJpgDec.setCallback(jpgRender);
    TJpgDec.drawFsJpg(390, 125, teslaLogo2);
}

void setup() {
    Serial.begin(DEFAULT_BAUD_RATE);
    // Build-in LED
    pinMode(ESP32C3_LED, OUTPUT);

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed!");
        return;
    }

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_TRANSPARENT);
    tft.setSwapBytes(true);

    loadBootLogo();
    delay(2000);

    // Initialize and draw the button
    initialMainPage();

    // Connect WIFI
    setup_Wifi();
    setupTimeZone();

    createStartButton();

    tft.setTextSize(1.9);

    if (WiFi.status() == WL_CONNECTED) {
        webSocket.begin(SOCKETIO_HOST, SOCKETIO_PORT);
        webSocket.on(SOCKETIO_CHANNEL, event);

        timer.every(3000, updateDisplayTeslaMate);
        timer.every(3000, updateDisplayTeslaWallConnect);
        //  wdt_enable(10000);
    }
}

Button getButton() {
    if (smartPress(tft, 400, 148, 20)) return BTN_START;
    if (smartPress(tft, 267, 260, 40)) return BTN_EXIT;
    return BTN_NONE;
}
void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        webSocket.loop();
        // wdt_reset();
    }

    switch (getButton()) {
        case BTN_START:
            Serial.println("START PRESSED!");
            isStarted = true;
            addCarLogo();
            break;
        case BTN_EXIT:
            Serial.println("EXIT PRESSED!");
            tft.fillScreen(TFT_BLACK);
            isStarted = false;

            initialMainPage();
            createStartButton();
            break;
    }

    timer.tick();
}
