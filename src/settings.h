#ifndef CONFIGS_H
#define CONFIGS_H

// config parameters
#define DEVICE_NAME "Tesla-WallConnector"
#define WIFI_SSID "MY_WIFI_SSID"
#define WIFI_PASSWORD "1234567890"
#define SOCKETIO_HOST "192.168.137.102"  // SocketIO host
#define SOCKETIO_PORT 4000
#define SOCKETIO_CHANNEL "ESP"
#define TESLA_WALLCONNECT_API "http://192.168.137.71/api/1/"  // TeslaWallConnect API
#define TESLAMATE_URL "http://192.168.137.21:4000"            // TeslaMate URL

// Line config
#define LINE_TOKEN "__YOUR_LINE_TOKEN___"
#define DEFAULT_BAUD_RATE 115200
#define API_TIMEOUT 3000
#define ENABLE_SOCKETIO true
#define ENABLE_DEBUG_MODE true
#define ENABLE_LINE_NOTIFY true

// #define EXT_WDT_PIN D5

#endif