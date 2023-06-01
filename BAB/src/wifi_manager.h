#ifndef WIFI_MANAGER.H
#define WIFI_MANAGER.H

#include <Arduino.h>

#define MAX_FIELD_SIZE  64

enum idle_client_server {
  IDLE,
  HOTSPOT,
  CLIENT  
};

enum wifi_state_enum {
  WIFI_NO_STATE,
  HOTSPOT_NOT_CONNECTED,
  HOTSPOT_CONNECTED,
  WIFI_NOT_CONNECTED,
  WIFI_CONNECTED
};

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
};

/**
 * @brief initialyze the wifi manager
 * 
 * this function check if this is the first boot of the device
 * and fill the settings accordingly, default or from eeprom
 * 
 * @param settings_ctx, wifi_idle_client_server : type of connection
*/
void init_wifi_management(settings_context *settings_ctx, idle_client_server *wifi_idle_client_server);

/**
 * @brief state machine of the wifi manager
 * 
 * init connexion or web managing
 * 
 * @param settings_ctx, wifi_idle_client_server : type of connection
*/
void wifi_management(settings_context *settings_ctx, idle_client_server *wifi_idle_client_server);

#endif //WIFI_MANAGER.H