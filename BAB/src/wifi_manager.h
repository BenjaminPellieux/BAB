/*
  Credits : Pierre BABIAN  pierrebabian4@gmail.com
  This is a script for wifi management on esp32
  
  - I used SimpleWiFiServer and WiFiAccessPoint from WiFi examples  -
  - And libraries that it is using                  -
*/


#ifndef WIFI_MANAGER
  #define WIFI_MANAGER

  #include <Arduino.h>

  #define MAX_FIELD_SIZE  64

  #include <EEPROM.h>//https://github.com/espressif/arduino-esp32/tree/master/libraries/EEPROM
  #include <WiFi.h>
  #include <WiFiAP.h>
  #include <string>


    
  #define EEPROM_SIZE sizeof(settings_context) + 1

  //errors :
  #define UNVALID_SETTING 1
  #define UNVALID_CHECK_SAVE  2

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
    float sensitivity; //[0;2]
    bool  mode_jauge_smiley;

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