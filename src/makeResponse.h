#ifndef MAKE_RESPONSE_H
#define MAKE_RESPONSE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SocketIoClient.h>

#include "Models/teslaApiReponse.h"
#include "settings.h"
#include "utility.h"
#include "wifiMan.h"

void createResponse(SocketIoClient& webSocket, TeslaApiReponse data) {
    StaticJsonDocument<768> root;

    root["deviceName"] = DEVICE_NAME;
    root["deviceId"] = getChipId();
    root["lastUpdated"] = NowString();
    root["ipAddress"] = WiFi.localIP().toString();
    root["wifiSignal"] = wifiSignal();

    JsonObject version = root.createNestedObject("version");
    version["firmware_version"] = data.firmware_version;
    version["git_branch"] = data.git_branch;
    version["part_number"] = data.part_number;
    version["serial_number"] = data.serial_number;
    version["web_service"] = data.web_service;

    JsonObject wifi_status = root.createNestedObject("wifi_status");
    wifi_status["wifi_ssid"] = data.wifi_ssid;
    wifi_status["wifi_signal_strength"] = data.wifi_signal_strength;
    wifi_status["wifi_rssi"] = data.wifi_rssi;
    wifi_status["wifi_snr"] = data.wifi_snr;
    wifi_status["wifi_connected"] = data.wifi_connected;
    wifi_status["wifi_infra_ip"] = data.wifi_infra_ip;
    wifi_status["internet"] = data.internet;
    wifi_status["wifi_mac"] = data.wifi_mac;

    JsonObject vitals = root.createNestedObject("vitals");
    vitals["contactor_closed"] = data.contactor_closed;
    vitals["vehicle_connected"] = data.vehicle_connected;
    vitals["session_s"] = data.session_s;
    vitals["grid_v"] = data.grid_v;
    vitals["grid_hz"] = data.grid_hz;
    vitals["vehicle_current_a"] = data.vehicle_current_a;
    vitals["currentA_a"] = data.currentA_a;
    vitals["currentB_a"] = data.currentB_a;
    vitals["currentC_a"] = data.currentC_a;
    vitals["currentN_a"] = data.currentN_a;
    vitals["voltageA_v"] = data.voltageA_v;
    vitals["voltageB_v"] = data.voltageB_v;
    vitals["voltageC_v"] = data.voltageC_v;
    vitals["relay_coil_v"] = data.relay_coil_v;
    vitals["pcba_temp_c"] = data.pcba_temp_c;
    vitals["handle_temp_c"] = data.handle_temp_c;
    vitals["mcu_temp_c"] = data.mcu_temp_c;
    vitals["uptime_s"] = data.uptime_s;
    vitals["input_thermopile_uv"] = data.input_thermopile_uv;
    vitals["prox_v"] = data.prox_v;
    vitals["pilot_high_v"] = data.pilot_high_v;
    vitals["pilot_low_v"] = data.pilot_low_v;
    vitals["session_energy_wh"] = data.session_energy_wh;
    vitals["config_status"] = data.config_status;
    vitals["evse_state"] = data.evse_state;

    JsonObject lifetime = root.createNestedObject("lifetime");
    lifetime["contactor_cycles"] = data.contactor_cycles;
    lifetime["contactor_cycles_loaded"] = data.contactor_cycles_loaded;
    lifetime["alert_count"] = data.alert_count;
    lifetime["thermal_foldbacks"] = data.thermal_foldbacks;
    lifetime["avg_startup_temp"] = data.avg_startup_temp;
    lifetime["charge_starts"] = data.charge_starts;
    lifetime["energy_wh"] = data.energy_wh;
    lifetime["connector_cycles"] = data.connector_cycles;
    lifetime["uptime_l_s"] = data.uptime_l_s;
    lifetime["charging_time_s"] = data.charging_time_s;

    String output;
    serializeJsonPretty(root, output);

    // Publish to socket.io server
    if (ENABLE_SOCKETIO)
        webSocket.emit(SOCKETIO_CHANNEL, output.c_str());

    if (ENABLE_DEBUG_MODE)
        Serial.print(output);
}

String delimiter = "|";
void publishMessages(TeslaApiReponse data) {
    String deviceName = DEVICE_NAME;
    String payload = delimiter + deviceName;

    payload.concat(delimiter + data.firmware_version);
    payload.concat(delimiter + data.grid_v);
    payload.concat(delimiter + data.grid_hz);
    payload.concat(delimiter + data.energy_wh);
    payload.concat(delimiter + data.vehicle_current_a);
    payload.concat(delimiter + "END");
    payload = String(strlen(payload.c_str())) + payload;

    // int len = strlen(payload.c_str());
    // Serial.println("Send Data: " + payload + " =>" + String(len));

    Serial.println(payload);
    Serial.flush();
}

#endif