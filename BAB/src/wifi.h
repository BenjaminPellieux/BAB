/*
  Credits : Pierre BABIAN  pierrebabian4@gmail.com
  This is a script for wifi management on esp32
  
  - I used SimpleWiFiServer and WiFiAccessPoint from WiFi examples  -
  - And libraries that it is using                  -
*/
#ifndef WIFI_DEFINED
#include <EEPROM.h>//https://github.com/espressif/arduino-esp32/tree/master/libraries/EEPROM
#include <WiFi.h>
//#include <WiFiClient.h>
#include <WiFiAP.h>
#include <string>
//#include <WebServer.h>
  
#define EEPROM_SIZE sizeof(settings_context) + 1
#define MAX_FIELD_SIZE  64

//errors :
#define UNVALID_SETTING 1
#define UNVALID_CHECK_SAVE  2
#define WIFI_DEFINED


enum idle_client_server {
  IDLE,
  HOTSPOT,
  CLIENT  
};

//wifi_state :
// - hotspot_not_connected -> hotspot is enabled, no client
// - hotspot_connected -> hotspot enabled, client connected -> client_idle or client_choosing
// - wifi_not_connected -> try to connect to wifi each n loops and hotspot enabled -> this state can lead to wifi_connected or hotspot_connected
// - wifi_connected -> wifi connected -> client_idle or client_choosing

enum wifi_state_enum {
  WIFI_NO_STATE,
  HOTSPOT_NOT_CONNECTED,
  HOTSPOT_CONNECTED,
  WIFI_NOT_CONNECTED,
  WIFI_CONNECTED
};

// Set these to your desired credentials.
const char *ssid = "BAB_0";
const char *password = "BAB_bpmp";
int count = 0;

WiFiServer server(80);

//context (saved in eeprom): 
struct settings_context {
  uint16_t brightness; //0 - 255
  uint16_t seuil_1;
  uint16_t seuil_2;
  uint32_t sensitivity; //0 - 255
  uint8_t mode_jauge_smiley;

  wifi_state_enum state_wifi;
  uint8_t connect_type; //hotspot, wifi
  char wifi_ssid[MAX_FIELD_SIZE];
  char wifi_pass[MAX_FIELD_SIZE];
  //wifi secu type ?
};

#endif