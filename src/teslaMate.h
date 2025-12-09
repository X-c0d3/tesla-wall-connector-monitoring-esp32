
#ifndef TESLAMATE_H
#define TESLAMATE_H

#include <Arduino.h>
#include <HTTPClient.h>

#include "settings.h"

struct TeslaMateResponse {
    float range_rated = 0;      // km
    float range_estimated = 0;  // km
    int charge_limit = 0;       // %
    int soc = 0;                // %
    float temp_outside = 0;     // °C
    float temp_inside = 0;      // °C
    int mileage = -1;           // km
    float speed = -1.0;         // km
    String estimated_range_100 = "";
    String status = "";
    String version = "";
};

TeslaMateResponse tesla;

String extractValue_withTooltip(const String& html, const String& label, String& tooltip) {
    tooltip = "";

    String search = "<td class=\"has-text-weight-medium\">" + label + "</td>";
    int p1 = html.indexOf(search);
    if (p1 == -1) return "";

    int p2 = html.indexOf("<td>", p1 + search.length());
    if (p2 == -1) return "";
    p2 += 4;

    int p3 = html.indexOf("</td>", p2);
    if (p3 == -1) return "";

    String raw = html.substring(p2, p3);

    int tipStart = raw.indexOf("data-tooltip=\"");
    if (tipStart != -1) {
        tipStart += 14;
        int tipEnd = raw.indexOf("\"", tipStart);
        if (tipEnd != -1) {
            tooltip = raw.substring(tipStart, tipEnd);
            tooltip.replace("&nbsp;", " ");
        }
    }

    while (raw.indexOf("<") != -1) {
        int start = raw.indexOf("<");
        int end = raw.indexOf(">", start);
        if (end == -1) break;
        raw.remove(start, end - start + 1);
    }

    raw.trim();
    return raw;
}
String extractValue(const String& html, const String& label) {
    String dummy;
    return extractValue_withTooltip(html, label, dummy);
}

TeslaMateResponse convertToTeslaMate(String html) {
    html.replace("\r", "");
    html.replace("\n", "");
    html.replace("&nbsp;", " ");
    while (html.indexOf("  ") != -1) html.replace("  ", " ");

    String val, tip;

    if (html.indexOf("Charge Limit") != -1) {
        val = extractValue(html, "Charge Limit");
        if (val.indexOf("%") != -1) {
            val.remove(val.indexOf("%"));
            val.trim();
            tesla.charge_limit = val.toInt();
        }
    }

    val = extractValue(html, "Status");
    if (val.length() > 0) {
        val.replace("&nbsp;", " ");
        tesla.status = val;
    }

    val = extractValue(html, "Speed");
    if (val.indexOf("km/h") != -1) {
        val.remove(val.indexOf("km/h"));
        val.replace("&nbsp;", " ");
        val.trim();
        tesla.speed = val.toFloat();
    }

    String soc_tooltip = "";
    val = extractValue_withTooltip(html, "State of Charge", soc_tooltip);
    if (val.length() > 0 && val.indexOf("%") != -1) {
        val.remove(val.indexOf("%"));
        val.trim();
        tesla.soc = val.toInt();
        tesla.estimated_range_100 = soc_tooltip;
    }

    int pos = html.indexOf("Range (rated)");
    if (pos != -1) {
        int td2 = html.indexOf("<td>", pos);
        if (td2 != -1) td2 = html.indexOf("<td>", td2 + 4);
        if (td2 != -1) {
            td2 += 4;
            int end = html.indexOf("</td>", td2);
            if (end != -1) {
                val = html.substring(td2, end);
                val.trim();
                int space = val.lastIndexOf(" ");
                if (space != -1) val = val.substring(0, space);
                if (val.length() > 0) tesla.range_rated = val.toFloat();
            }
        }
    }

    val = extractValue(html, "Range (est.)");
    int km = val.lastIndexOf("km");
    if (km == -1) km = val.lastIndexOf(" ");
    if (km != -1) {
        val = val.substring(0, km);
        val.trim();
        if (val.length() > 0) tesla.range_estimated = val.toFloat();
    }

    val = extractValue(html, "Outside Temperature");
    int deg = val.indexOf("°");
    if (deg != -1) {
        val = val.substring(0, deg);
        val.trim();
        tesla.temp_outside = val.toFloat();
    }

    val = extractValue(html, "Inside Temperature");
    deg = val.indexOf("°");
    if (deg != -1) {
        val = val.substring(0, deg);
        val.trim();
        tesla.temp_inside = val.toFloat();
    }

    val = extractValue(html, "Mileage");
    km = val.lastIndexOf("km");
    if (km != -1) {
        val = val.substring(0, km);
        val.replace(",", "");
        val.trim();
        tesla.mileage = val.toInt();
    }

    int p = html.indexOf("software-updates/version/");
    if (p != -1) {
        p += 25;
        int end = html.indexOf("/", p);
        if (end == -1) end = html.indexOf("\"", p);
        if (end != -1) {
            tesla.version = html.substring(p, end);
        }
    }

    return tesla;
}

void printTeslaMateResults() {
    if (ENABLE_DEBUG_MODE) {
        Serial.println("════════════════════════");
        Serial.printf("Status              : %s\n", tesla.status.c_str());
        Serial.printf("Range (rated)       : %.2f km\n", tesla.range_rated);
        Serial.printf("Range (est.)        : %.2f km\n", tesla.range_estimated);
        Serial.printf("Charge Limit        : %d%%\n", tesla.charge_limit);
        Serial.printf("State of Charge     : %d%%\n", tesla.soc);
        if (tesla.speed >= 0) {
            Serial.printf("Speed               : %.0f km/h\n", tesla.speed);
        }
        Serial.printf("Est. Range @100%%    : %s\n", tesla.estimated_range_100.c_str());
        Serial.printf("Outside Temp        : %.1f °C\n", tesla.temp_outside);
        Serial.printf("Inside Temp         : %.1f °C\n", tesla.temp_inside);
        Serial.printf("Mileage             : %d km\n", tesla.mileage);
        Serial.printf("Version             : %s\n", tesla.version);
        Serial.println("════════════════════════\n");
    }
}

#endif