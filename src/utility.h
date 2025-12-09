
#ifndef UTILITY_H
#define UTILITY_H

#include <Arduino.h>
#include <HTTPClient.h>

int timezone = 7;
char ntp_server1[20] = "ntp.ku.ac.th";
char ntp_server2[20] = "fw.eng.ku.ac.th";
char ntp_server3[20] = "time.uni.net.th";
int dst = 0;

void printMessage(int X, int Y, String message, bool isPrintLn) {
    if (isPrintLn)
        Serial.println(message);
    else
        Serial.print(message);
}

String NowString() {
    time_t now = time(nullptr);
    struct tm* newtime = localtime(&now);
    String tmpNow = "";
    tmpNow += String(newtime->tm_hour);
    tmpNow += ":";
    tmpNow += String(newtime->tm_min);
    tmpNow += ":";
    tmpNow += String(newtime->tm_sec);
    return tmpNow;
}

void setupTimeZone() {
    configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
    Serial.println("Waiting for time");
    while (!time(nullptr)) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("Curent time: " + NowString());
}

String getSplitValue(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }

    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String getChipId() {
    String ChipIdHex = String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
    ChipIdHex += String((uint32_t)ESP.getEfuseMac(), HEX);
    return ChipIdHex;
}

String smartEnergyW(float wh) {
    if (wh < 1000.0) {
        return String(wh, 1) + " Wh";
    } else if (wh < 1000000.0) {
        float kwh = wh / 1000.0;
        return String(kwh, 2) + " kWh";
    } else {
        float mwh = wh / 1000000.0;
        return String(mwh, 3) + " MWh";
    }
}

String smartDuration(long sec) {
    if (sec < 60) {
        return String(sec) + " sec";
    } else if (sec < 3600) {
        float minutes = sec / 60.0;
        return String(minutes, minutes < 10 ? 1 : 0) + " min";
    } else if (sec < 86400) {
        float hours = sec / 3600.0;
        return String(hours, hours < 10 ? 1 : 0) + " hr";
    } else {
        float days = sec / 86400.0;
        return String(days, days < 10 ? 1 : 0) + " days";
    }
}

void drawBar(TFT_eSPI tft, int x, int y, int w, int h, float value, float maxValue, uint16_t color) {
    int filled = (int)((value / maxValue) * w);
    tft.fillRect(x, y, w, h, TFT_DARKGREY);
    tft.fillRect(x, y, filled, h, color);
}

void drawFixedText(TFT_eSPI tft, int x, int y, int width, int height, String text, uint16_t textColor, uint16_t bgColor, uint8_t font = 2) {
    tft.fillRect(x, y, width, height, bgColor);

    tft.setTextColor(textColor, bgColor);
    tft.setTextFont(font);
    tft.setCursor(x, y);

    tft.print(text);
}

void drawStatus(TFT_eSPI tft, int x, int y, bool status) {
    tft.fillCircle(x, y, 6, status ? TFT_GREEN : TFT_RED);
}

String httpGet(String url) {
    HTTPClient http;
    http.begin(url);
    int code = http.GET();
    if (code == HTTP_CODE_OK) {
        String payload = http.getString();
        http.end();
        return payload;
    }
    http.end();
    Serial.println("HTTP GET failed: " + String(code));
    return "";
}

#endif