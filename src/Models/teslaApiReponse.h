

#ifndef TeslaApi_Reponse
#define TeslaApi_Reponse

#include <ArduinoJson.h>

#include "Arduino.h"
#include "settings.h"
class TeslaApiReponse {
   public:
    // version
    String firmware_version;
    String git_branch;
    String part_number;
    String serial_number;
    String web_service;
    // wifi_status
    String wifi_ssid;
    float wifi_signal_strength;
    float wifi_rssi;
    float wifi_snr;
    bool wifi_connected;
    String wifi_infra_ip;
    bool internet;
    String wifi_mac;
    // vitals
    bool contactor_closed;
    bool vehicle_connected;
    long session_s;
    float grid_v;
    float grid_hz;
    float vehicle_current_a;
    float currentA_a;
    float currentB_a;
    float currentC_a;
    float currentN_a;
    float voltageA_v;
    float voltageB_v;
    float voltageC_v;
    float relay_coil_v;
    float pcba_temp_c;
    float handle_temp_c;
    float mcu_temp_c;
    long uptime_s;
    float input_thermopile_uv;
    float prox_v;
    float pilot_high_v;
    float pilot_low_v;
    float session_energy_wh;
    int config_status;
    int evse_state;
    // lifetime
    int contactor_cycles;
    int contactor_cycles_loaded;
    int alert_count;
    int thermal_foldbacks;
    float avg_startup_temp;
    int charge_starts;
    long energy_wh;
    int connector_cycles;
    long uptime_l_s;
    long charging_time_s;
};

void convertFromJson(JsonVariantConst version, JsonVariantConst wifi_status, JsonVariantConst vitals, JsonVariantConst lifetime, TeslaApiReponse& data) {
    // version
    data.firmware_version = version["firmware_version"].as<String>();
    data.git_branch = version["git_branch"].as<String>();
    data.part_number = version["part_number"].as<String>();
    data.serial_number = version["serial_number"].as<String>();
    data.web_service = version["web_service"].as<String>();
    // wifi_status
    data.wifi_ssid = wifi_status["wifi_ssid"].as<String>();
    data.wifi_signal_strength = wifi_status["wifi_signal_strength"].as<float>();
    data.wifi_rssi = wifi_status["wifi_rssi"].as<float>();
    data.wifi_snr = wifi_status["wifi_snr"].as<float>();
    data.wifi_connected = wifi_status["wifi_connected"].as<bool>();
    data.wifi_infra_ip = wifi_status["wifi_infra_ip"].as<String>();
    data.internet = wifi_status["internet"].as<bool>();
    data.wifi_mac = wifi_status["wifi_mac"].as<String>();
    // vitals
    data.contactor_closed = vitals["contactor_closed"].as<bool>();
    data.vehicle_connected = vitals["vehicle_connected"].as<bool>();
    data.session_s = vitals["session_s"].as<long>();
    data.grid_v = vitals["grid_v"].as<float>();
    data.grid_hz = vitals["grid_hz"].as<float>();
    data.vehicle_current_a = vitals["vehicle_current_a"].as<float>();
    data.currentA_a = vitals["currentA_a"].as<float>();
    data.currentB_a = vitals["currentB_a"].as<float>();
    data.currentC_a = vitals["currentC_a"].as<float>();
    data.currentN_a = vitals["currentN_a"].as<float>();
    data.voltageA_v = vitals["voltageA_v"].as<float>();
    data.voltageB_v = vitals["voltageB_v"].as<float>();
    data.voltageC_v = vitals["voltageC_v"].as<float>();
    data.relay_coil_v = vitals["relay_coil_v"].as<float>();
    data.pcba_temp_c = vitals["pcba_temp_c"].as<float>();
    data.handle_temp_c = vitals["handle_temp_c"].as<float>();
    data.mcu_temp_c = vitals["mcu_temp_c"].as<float>();
    data.uptime_s = vitals["uptime_s"].as<long>();
    data.input_thermopile_uv = vitals["input_thermopile_uv"].as<float>();
    data.prox_v = vitals["prox_v"].as<float>();
    data.pilot_high_v = vitals["pilot_high_v"].as<float>();
    data.pilot_low_v = vitals["pilot_low_v"].as<float>();
    data.session_energy_wh = vitals["session_energy_wh"].as<float>();
    data.config_status = vitals["config_status"].as<int>();
    data.evse_state = vitals["evse_state"].as<int>();
    // lifetime
    data.contactor_cycles = lifetime["contactor_cycles"].as<int>();
    data.contactor_cycles_loaded = lifetime["contactor_cycles_loaded"].as<int>();
    data.alert_count = lifetime["alert_count"].as<int>();
    data.thermal_foldbacks = lifetime["thermal_foldbacks"].as<int>();
    data.avg_startup_temp = lifetime["avg_startup_temp"].as<float>();
    data.charge_starts = lifetime["charge_starts"].as<int>();
    data.energy_wh = lifetime["energy_wh"].as<long>();
    data.connector_cycles = lifetime["connector_cycles"].as<int>();
    data.uptime_l_s = lifetime["uptime_s"].as<long>();
    data.charging_time_s = lifetime["charging_time_s"].as<long>();

    if (ENABLE_DEBUG_MODE) {
        Serial.println("=== Tesla Wall Connector ===");
        Serial.println("Firmware: " + String(data.firmware_version));
        Serial.println("Serial: " + String(data.serial_number));

        Serial.println("Vehicle connected: " + String(data.vehicle_connected ? "Yes" : "No"));
        Serial.println("Contactor closed: " + String(data.contactor_closed ? "Yes" : "No"));
        Serial.println("Session: " + String(data.session_s) + " sec");
        Serial.println("Grid AC: " + String(data.grid_v) + " V");
        Serial.println("Frequency: " + String(data.grid_hz) + " Hz");
        Serial.println("CurrentA: " + String(data.currentA_a) + "A");
        Serial.println("CurrentB: " + String(data.currentB_a) + "A");
        Serial.println("CurrentC: " + String(data.currentC_a) + "A");
        Serial.println("CurrentN: " + String(data.currentN_a) + "A");
        Serial.println("VoltageA: " + String(data.voltageA_v) + "V");
        Serial.println("VoltageB: " + String(data.voltageB_v) + "V");
        Serial.println("VoltageC: " + String(data.voltageC_v) + "V");
        Serial.println("Vehicle Current: " + String(data.vehicle_current_a) + " A");
        Serial.println("PCBA Temp: " + String(data.pcba_temp_c) + " C");
        Serial.println("Handle Temp: " + String(data.handle_temp_c) + " C");
        Serial.println("Session energy: " + String(data.session_energy_wh) + " Wh");
        Serial.println("Input_thermopile_uv: " + String(data.input_thermopile_uv));
        Serial.println("Prox: " + String(data.prox_v) + +" V");
        Serial.println("Pilot_high: " + String(data.pilot_high_v) + " V");
        Serial.println("Pilot_low: " + String(data.pilot_low_v) + " V");
        Serial.println("State: " + String(data.evse_state));

        Serial.println("Contactor cycles: " + String(data.contactor_cycles));
        Serial.println("Energy: " + String(data.energy_wh / 1000.0) + " kWh");

        float uptimeDays = data.uptime_s / 86400;
        Serial.println("Uptime: " + String(uptimeDays, 0) + " days");

        Serial.println("SSID: " + String(data.wifi_ssid));
        Serial.println("RSSI: " + String(data.wifi_rssi) + " dBm");
        Serial.println("Internet: " + String(data.internet ? "Yes" : "No"));
        Serial.println("==============================");
    }
}

bool canConvertFromJson(JsonVariantConst src, const TeslaApiReponse&) {
    return src.is<JsonObject>();
}

#endif