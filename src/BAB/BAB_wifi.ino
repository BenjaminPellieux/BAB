/*
  Credits : Pierre BABIAN  pierrebabian4@gmail.com
  This is a script for wifi management on esp32
  
  - I used SimpleWiFiServer and WiFiAccessPoint from WiFi examples  -
  - And libraries that it is using                  -
*/
#include <EEPROM.h>//https://github.com/espressif/arduino-esp32/tree/master/libraries/EEPROM
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WebServer.h>

  
#define EEPROM_SIZE 12
#define MAX_FIELD_SIZE  64

//errors :
#define UNVALID_SETTING 1
#define UNVALID_CHECK_SAVE  2

//addresses : 
#define SEUIL_1_ADRESS    0
#define SEUIL_2_ADRESS    2
#define BRIGHTNESS_ADRESS 4
#define SENSIVITY_ADRESS  6
#define MODE_JAUGE_SMILEY_ADRESS 10
#define STATE_WIFI_ADRESS 11
#define SSID_ADRESS       13
#define PWD_ADRESS        MAX_FIELD_SIZE + SSID_ADRESS

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
  //...
  wifi_state_enum state_wifi_on_boot;
  uint8_t connect_type; //hotspot, wifi
  char wifi_ssid[MAX_FIELD_SIZE];
  char wifi_pass[MAX_FIELD_SIZE];
  //wifi secu type ?
  //...  
};

void page_hotspot_string(String *string_html, settings_context settings) {
*string_html += "\n";
//*string_html += "HTTP/1.1 200 OK";
//*string_html += "Content-type:text/html";
*string_html += "<!DOCTYPE html>";
*string_html += "<html>";
*string_html += "<head>";
*string_html += "  <title>Formulaire de configuration</title>";
*string_html += "  <script type='text/javascript'>";
 // Script pour exclure mutuellement les cases à cocher";
*string_html += "    function toggleSmileyJauge() {";
*string_html += "      if (document.getElementById('jauge').checked) {";
*string_html += "        document.getElementById('smiley').checked = false;";
*string_html += "      } else if (document.getElementById('smiley').checked) {";
*string_html += "        document.getElementById('jauge').checked = false;";
*string_html += "      }";

*string_html += "    }";
*string_html += "  </script>";
*string_html += "</head>";
*string_html += "<body>";
*string_html += "  <h2>Configuration du systeme</h2>";
*string_html += "  <form action='/' method='post'>";
*string_html += "    <label for='input1'>Ssid :</label>";
*string_html += "    <input type='text' id='input1' name='input1' value='"; *string_html += settings.wifi_ssid; *string_html += "'><br><br>";

*string_html += "    <label for='input2'>Mot de passe :</label>";
*string_html += "    <input type='text' id='input2' name='input2' value='"; *string_html += settings.wifi_pass; *string_html += "'><br><br>";

*string_html += "    <label for='seuilHaut'>Seuil haut :</label>";
*string_html += "    <input type='range' id='seuilHaut' name='seuilHaut' min='0' max='255' value='"; *string_html += String(settings.seuil_1); *string_html += "' onchange='document.getElementById('valueSeuilHaut').innerHTML = this.value'><span id='valueSeuilHaut'></span><br><br>";

*string_html += "    <label for='seuilBas'>Seuil bas :</label>";
*string_html += "    <input type='range' id='seuilBas' name='seuilBas' min='0' max='255' value='"; *string_html += String(settings.seuil_2); *string_html += "' onchange='document.getElementById('valueSeuilBas').innerHTML = this.value'><span id='valueSeuilBas'></span><br><br>";

*string_html += "    <label for='sensibilite'>Sensibilite :</label>";
*string_html += "    <input type='range' id='sensibilite' name='sensibilite' min='0' max='255' value='"; *string_html += String(settings.sensitivity); *string_html += "' onchange='document.getElementById('valueSensibilite').innerHTML = this.value'><span id='valueSensibilite'></span><br><br>";
    
*string_html += "    <label for='luminosite'>Luminosite :</label>";
*string_html += "    <input type='range' id='luminosite' name='luminosite' min='0' max='255' value='"; *string_html += String(settings.brightness); *string_html += "' onchange='document.getElementById('valueLuminosite').innerHTML = this.value'><span id='valueLuminosite'></span><br><br>";
    
*string_html += "    <label for='smiley'>Smiley :</label>";
*string_html += "    <input type='checkbox' id='smiley' name='smiley' value='"; *string_html += settings.mode_jauge_smiley; *string_html += "' onclick='toggleSmileyJauge()'><br><br>";
    
*string_html += "    <label for='jauge'>Jauge :</label>";
*string_html += "    <input type='checkbox' id='jauge' name='jauge' value='"; *string_html += !settings.mode_jauge_smiley; *string_html += "' onclick='toggleSmileyJauge()'><br><br>";
    
*string_html += "    <input type='submit' value='Envoyer'>";
*string_html += "  </form>";
*string_html += "  <script type='text/javascript'>";
    // Initialisation des valeurs des jauges
*string_html += "    document.getElementById('valueSeuilHaut').innerHTML = document.getElementById('seuilHaut').value;";
*string_html += "    document.getElementById('valueSeuilBas').innerHTML = document.getElementById('seuilBas').value;";
*string_html += "    document.getElementById('valueSensibilite').innerHTML = document.getElementById('sensibilite').value;";
*string_html += "    document.getElementById('valueLuminosite').innerHTML = document.getElementById('luminosite').value;";
*string_html += "  </script>";
*string_html += "</body>";
*string_html += "</html>";
*string_html += "\n";
*string_html += "HTTP/1.1 200 OK";
*string_html += "\n";
//Serial.println(*string_html);
}

int get_settings_ctx(settings_context *settings_ctx)
{
  int ret = 0;
  int address = MODE_JAUGE_SMILEY_ADRESS;
  settings_ctx->mode_jauge_smiley = EEPROM.readUShort(address);
  if ((settings_ctx->mode_jauge_smiley > 255) || (settings_ctx->mode_jauge_smiley < 0)) {
    ret = -UNVALID_SETTING;
  }
  
  address = SEUIL_1_ADRESS;
  settings_ctx->seuil_1 = EEPROM.readUShort(address);
  if ((settings_ctx->seuil_1 > 255) || (settings_ctx->seuil_1 < 0)) {
    ret = -UNVALID_SETTING;
  }
  
  address = SEUIL_2_ADRESS;
  settings_ctx->seuil_2 = EEPROM.readUShort(address);
  if ((settings_ctx->seuil_2 > 255) || (settings_ctx->seuil_2 < 0)) {
    ret = -UNVALID_SETTING;
  }
  
  address = BRIGHTNESS_ADRESS;
  settings_ctx->brightness = EEPROM.readUShort(address);
  if ((settings_ctx->brightness > 255) || (settings_ctx->brightness < 0)) {
    ret = -UNVALID_SETTING;
  }
  
  address = SENSIVITY_ADRESS;
  settings_ctx->sensitivity = EEPROM.readULong(address);
  if ((settings_ctx->sensitivity > 255) || (settings_ctx->sensitivity < 0)) {
    ret = -UNVALID_SETTING;
  }
  
  address = STATE_WIFI_ADRESS;
  settings_ctx->state_wifi_on_boot = (wifi_state_enum)EEPROM.readUShort(address);
  if ((settings_ctx->state_wifi_on_boot > WIFI_CONNECTED)
      || (settings_ctx->state_wifi_on_boot < HOTSPOT_NOT_CONNECTED)) {
    ret = -UNVALID_SETTING;
  }
  address = SSID_ADRESS;
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    settings_ctx->wifi_ssid[i] = EEPROM.readChar(address + i);
  }

  address = PWD_ADRESS;
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    settings_ctx->wifi_pass[i] = EEPROM.readChar(address + i);
  }
  return ret;
}



int save_settings_ctx(settings_context settings_ctx)
{
  int ret = 0;
  int address = MODE_JAUGE_SMILEY_ADRESS;
  EEPROM.writeByte(address, settings_ctx.mode_jauge_smiley);
  address = SEUIL_1_ADRESS;
  EEPROM.writeUShort(address, settings_ctx.seuil_1);
  address = SEUIL_2_ADRESS;
  EEPROM.writeUShort(address, settings_ctx.seuil_2);
  address = BRIGHTNESS_ADRESS;
  EEPROM.writeUShort(address, settings_ctx.brightness);
  address = SENSIVITY_ADRESS;
  EEPROM.writeULong(address, settings_ctx.sensitivity);
  address = STATE_WIFI_ADRESS;
  EEPROM.writeUShort(address, settings_ctx.state_wifi_on_boot);
  address = SSID_ADRESS;
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    EEPROM.writeChar(address + i, settings_ctx.wifi_ssid[i]);
  }
  address = PWD_ADRESS;
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    EEPROM.writeChar(address + i, settings_ctx.wifi_pass[i]);
  }
 
  EEPROM.commit();
  //verification
  address = MODE_JAUGE_SMILEY_ADRESS;
  settings_ctx.mode_jauge_smiley = EEPROM.readByte(address);
  if ((settings_ctx.mode_jauge_smiley > 255) || (settings_ctx.mode_jauge_smiley < 0)) {
    ret = -UNVALID_SETTING;
  }
  
  address = SEUIL_1_ADRESS;
  settings_ctx.seuil_1 = EEPROM.readUShort(address);
  if ((settings_ctx.seuil_1 > 255) || (settings_ctx.seuil_1 < 0)) {
    ret = -UNVALID_SETTING;
  }
  
  address = SEUIL_2_ADRESS;
  settings_ctx.seuil_2 = EEPROM.readUShort(address);
  if ((settings_ctx.seuil_2 > 255) || (settings_ctx.seuil_2 < 0)) {
    ret = -UNVALID_SETTING;
  }
  address = BRIGHTNESS_ADRESS;
  if (settings_ctx.brightness != EEPROM.readUShort(address)) {
    ret = -UNVALID_CHECK_SAVE;
  }
  address = SENSIVITY_ADRESS;
  if (settings_ctx.sensitivity != EEPROM.readULong(address)) {
    ret = -UNVALID_CHECK_SAVE;
  }
  address = STATE_WIFI_ADRESS;
  if (settings_ctx.state_wifi_on_boot != EEPROM.readUShort(address)) {
    ret = -UNVALID_CHECK_SAVE;
  }
  
  address = SSID_ADRESS;
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    if (settings_ctx.wifi_ssid[i] != EEPROM.readChar(address + i)) {
      ret = -UNVALID_CHECK_SAVE;
    }
  }
  
  address = PWD_ADRESS;
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    if (settings_ctx.wifi_pass[i] != EEPROM.readChar(address + i)) {
      ret = -UNVALID_CHECK_SAVE;
    }
  }
  return ret;
}

void init_wifi_connect(settings_context *settings_ctx)
{
   Serial.println();
    Serial.print("Connecting to ");
    Serial.println(settings_ctx->wifi_ssid);

    WiFi.begin(settings_ctx->wifi_ssid, settings_ctx->wifi_pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
 
  server.begin();
}

void init_hotspot(settings_context *settings_ctx)
{
  //WiFi.softAP(settings_ctx->wifi_ssid, settings_ctx->wifi_pass);
  WiFi.softAP(settings_ctx->wifi_ssid, settings_ctx->wifi_pass); 
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();
  Serial.println("Server started");
}

void user_management_wifi_connect(wifi_state_enum *state_wifi, settings_context *settings_ctx)
{
  
}

void user_management_hotspot(wifi_state_enum *state_wifi, settings_context *settings_ctx)
{
  String string_html;
  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {
          Serial.println("DEBUG 1.3");
          //Serial.print(currentLine);
          if (currentLine.indexOf("POST /submit_1") > 0) {
            // Read incoming POST data
            Serial.println("DEBUG 1.1");
            String line = client.readStringUntil('\r');
            Serial.print("DEBUG : client read : ");
            Serial.println(line);
            if (line.startsWith("seuilhaut")) {
              settings_ctx->seuil_1 = line.substring(line.indexOf('=') + 1).toInt();
            } else if (line.startsWith("seuilbas")) {
              settings_ctx->seuil_2 = line.substring(line.indexOf('=') + 1).toInt();
            } else if (line.startsWith("sensibilite")) {
              settings_ctx->sensitivity = line.substring(line.indexOf('=') + 1).toInt();
            } else if (line.startsWith("luminosite")) {
              settings_ctx->brightness = line.substring(line.indexOf('=') + 1).toInt();
            } else if (line.startsWith("smiley")) {
              settings_ctx->mode_jauge_smiley = line.substring(line.indexOf('=') + 1).toInt();
            }
            Serial.println("Context :");
            Serial.println(settings_ctx->seuil_1);
          } else {    // if you got a newline, then clear currentLine:
            Serial.println("DEBUG 1.2");
            currentLine = "";
            //save_settings_ctx(*settings_ctx);
            // Send HTTP response with updated values
            page_hotspot_string(&string_html, *settings_ctx);
            client.println(string_html);

          }
          //read the end of the request (not used for now)
          while (client.available()){
            //do nothing until the string of the request has been received
            client.read();
          }
        } else {
          currentLine += c;
        }
      }
      client = server.available();
     }
    // close the connection:
    //client.stop();
    //Serial.println("Client Disconnected.");
  } else {
    count++;
    if (count > 100000) {
      count = 0;
      Serial.println("Searching for client");
    }
  }
}
void hotspot_connected(wifi_state_enum *state_wifi, settings_context *settings_ctx, uint16_t *wifi_idle_client_server)
{
    user_management_hotspot(state_wifi, settings_ctx);
}

void hotspot_not_connected(wifi_state_enum *state_wifi, settings_context *settings_ctx, uint16_t *wifi_idle_client_server)
{
  switch (*wifi_idle_client_server) {
  case CLIENT://case when the device is comming from wifi setup
    server.end();
    WiFi.disconnect();
    *wifi_idle_client_server = HOTSPOT;
  case IDLE:
  case HOTSPOT:
    init_hotspot(settings_ctx);
    *state_wifi = HOTSPOT_CONNECTED;
    Serial.println("Going in hotspot connected");
    break;
  }
}

void wifi_connected(wifi_state_enum *state_wifi, settings_context *settings_ctx, uint16_t *wifi_idle_client_server)
{
  if (WiFi.status() == WL_CONNECTED) {
    user_management_wifi_connect(state_wifi, settings_ctx);
  } else {
    *state_wifi = WIFI_NOT_CONNECTED;
  }
}

void wifi_not_connected(wifi_state_enum *state_wifi, settings_context *settings_ctx, uint16_t *wifi_idle_client_server)
{
  switch (*wifi_idle_client_server) {
  case HOTSPOT: //case when the device is comming from hotspot setup
    server.end();
    WiFi.disconnect();
    *wifi_idle_client_server = IDLE;
  case IDLE:
    init_wifi_connect(settings_ctx);
    *wifi_idle_client_server = CLIENT;
  case CLIENT:
    if (WiFi.status() == WL_CONNECTED) {
      *state_wifi = WIFI_CONNECTED;
    } else {
      //searching wifi router
    }
    break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  EEPROM.begin(EEPROM_SIZE);
}

//--------------------------------------------------------------------------------------//
//----------------------------------------MAIN------------------------------------------//
//--------------------------------------------------------------------------------------//

void loop() {
  //on boot :
  //getting contexts
  settings_context settings_ctx;
  get_settings_ctx(&settings_ctx);
  wifi_state_enum state_wifi = settings_ctx.state_wifi_on_boot;
  int loop = 1;
  int ret = 0;
  uint16_t wifi_idle_client_server = 0; //0 on idle, 1 on hotspot and 2 on server
  
  //TODO if first boot : default params
  //editing ctx and save it
  //end first boot


  while (loop) {
  //wifi state machine :
    switch (state_wifi) {
      case WIFI_NO_STATE:
      Serial.println("State wifi not defined !\nSetting up wifi and going to hotspot not connected");
      //if nothing in wifi settings (from 
      if (settings_ctx.wifi_ssid[0] == 0) {
        Serial.println("wifi setings default");
        //btw can be used for first boot
        for (int i = 0; i < MAX_FIELD_SIZE; i++) {
          settings_ctx.wifi_ssid[i] = ssid[i];
          settings_ctx.wifi_pass[i] = password[i];
        }
        ret = save_settings_ctx(settings_ctx);
        if (ret != 0) {
          Serial.println("save failed");
        }
        Serial.print("ssid : ");
        Serial.println(settings_ctx.wifi_ssid);
      }
      state_wifi = HOTSPOT_NOT_CONNECTED;
      break;
      
      case HOTSPOT_NOT_CONNECTED:
      hotspot_not_connected(&state_wifi, &settings_ctx, &wifi_idle_client_server);
      break;

      case HOTSPOT_CONNECTED:
      hotspot_connected(&state_wifi, &settings_ctx, &wifi_idle_client_server);
      break;      

      case WIFI_NOT_CONNECTED:
      wifi_not_connected(&state_wifi, &settings_ctx, &wifi_idle_client_server);
      break;

      case WIFI_CONNECTED:
      wifi_connected(&state_wifi, &settings_ctx, &wifi_idle_client_server);
      break;

      default:
      //
      break;
    }
  }
}

//--------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------//

//TODO :
//first boot
//get ssid and pasword from post response
//user managements -> web pages (using post requests for more security)
//be aware of the id/password overflow and code injections
