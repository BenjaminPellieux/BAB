
#include "wifi_manager.h"

const char *ssid = "BAB_0";
const char *password = "BAB_bpmp";
uint16_t count = 0;
WiFiServer server(80);

/**
 * @brief store the web page in a string
 * 
 * @param string_html the string where the web page will be stored, settings_ctx
*/
static void page_hotspot_string(String *string_html, settings_context settings) {
  *string_html += " <!DOCTYPE html> <html> <head> <title>Formulaire de configuration</title>";
  
  *string_html += " </head> <script type='text/javascript'> function seuil_value(e){"
            "  let seuilHaut = parseInt(document.getElementById('seuilHaut').value);"
            "  let seuilBas = parseInt(document.getElementById('seuilBas').value);"
            "  if ((e.id === 'seuilHaut') && (seuilHaut <= seuilBas)){document.getElementById('seuilBas').value = document.getElementById('seuilHaut').value;"
            "  }else if((e.id === 'seuilBas') && (seuilHaut <= seuilBas)){document.getElementById('seuilHaut').value = document.getElementById('seuilBas').value;}}"  
            "</script> <body> <h2>Configuration du systeme</h2> <form action='/' method='post'> <label for='input1'>Ssid :</label>";

  *string_html += "    <input type='text' id='input1' name='input1' value='"; *string_html += settings.wifi_ssid; *string_html += "'><br><br>";

  *string_html += "    <label for='input2'>Mot de passe :</label>";
  *string_html += "    <input type='password' id='input2' name='input2' value='"; *string_html += settings.wifi_pass; *string_html += "'><br><br>";

  *string_html += "    <label for='seuilHaut'>Seuil haut :</label>";
  *string_html += "    <input type='range' id='seuilHaut' name='seuilHaut' min='0' max='120' onchange='seuil_value(this)' value='"; *string_html += String(settings.seuil_1); *string_html += "'><br><br>";

  *string_html += "    <label for='seuilBas'>Seuil bas :</label>";
  *string_html += "    <input type='range' id='seuilBas' name='seuilBas' min='0' max='120' onchange='seuil_value(this)' value='"; *string_html += String(settings.seuil_2); *string_html += "'><br><br>";

  *string_html += "    <label for='sensibilite'>Sensibilite :</label>";
  *string_html += "    <input type='range' id='sensibilite' name='sensibilite' min='0' max='255' value='"; *string_html += String(settings.sensitivity*128); *string_html += "'><br><br>";
      
  *string_html += "    <label for='luminosite'>Luminosite :</label>";
  *string_html += "    <input type='range' id='luminosite' name='luminosite' min='0' max='255' value='"; *string_html += String(settings.brightness); *string_html += "'><br><br>";
  
  *string_html += "    <label for='jauge'>Jauge (else smiley) :</label>";
  *string_html += "    <input type='checkbox' id='jauge' name='jauge' "; *string_html += settings.mode_jauge_smiley ? "checked" : ""; *string_html += "><br><br>";

  *string_html += "    <label for='wifi_type'>Connect to ssid :</label>";
  *string_html += "    <input type='checkbox' id='connect_type' name='connect_type' "; *string_html += settings.connect_type ? "checked" : ""; *string_html += "><br><br>";
  *string_html += "    <input type='submit' value='Envoyer'> </form> </body> <html>";
  
}

/**
 * @brief get settings from eeprom
 * 
 * settings are read and verified to be sure that they are valid
 * 
 * @param settings_ctx
 * 
 * @return 0 if ok, else -UNVALID_SETTING
*/
static int get_settings_ctx(settings_context *settings_ctx)
{
  Serial.println("Get settings");
  int ret = 0;
  int address = 1;
  settings_ctx->mode_jauge_smiley = EEPROM.readByte(address);
  if ((settings_ctx->mode_jauge_smiley > 255) || (settings_ctx->mode_jauge_smiley < 0)) {
    ret = -UNVALID_SETTING;
  }
  
  address += sizeof(byte);
  settings_ctx->seuil_1 = EEPROM.readUShort(address);
  if ((settings_ctx->seuil_1 > 255) || (settings_ctx->seuil_1 < 0)) {
    ret = -UNVALID_SETTING;
  }
  
  address += sizeof(unsigned short);
  settings_ctx->seuil_2 = EEPROM.readUShort(address);
  if ((settings_ctx->seuil_2 > 255) || (settings_ctx->seuil_2 < 0)) {
    ret = -UNVALID_SETTING;
  }
  
  address += sizeof(unsigned short);
  settings_ctx->brightness = EEPROM.readUShort(address);
  if ((settings_ctx->brightness > 255) || (settings_ctx->brightness < 0)) {
    ret = -UNVALID_SETTING;
  }
  
  address += sizeof(unsigned short);
  settings_ctx->sensitivity = EEPROM.readULong(address);
  if ((settings_ctx->sensitivity > 255) || (settings_ctx->sensitivity < 0)) {
    ret = -UNVALID_SETTING;
  }
  
  address += sizeof(unsigned long);
  settings_ctx->connect_type = (wifi_state_enum)EEPROM.readUShort(address);
  if ((settings_ctx->connect_type > WIFI_CONNECTED)
      || (settings_ctx->connect_type < HOTSPOT_NOT_CONNECTED)) {
    ret = -UNVALID_SETTING;
  }
  
  address += sizeof(unsigned short);
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    settings_ctx->wifi_ssid[i] = EEPROM.readChar(address + i * sizeof(char));
    if (settings_ctx->wifi_ssid[i] > 255) {
      ret = -UNVALID_SETTING;
      break;
    }
  }

  address += MAX_FIELD_SIZE * sizeof(char);
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    settings_ctx->wifi_pass[i] = EEPROM.readChar(address + i * sizeof(char));
    if (settings_ctx->wifi_ssid[i] > 255) {
      ret = -UNVALID_SETTING;
      break;
    }
  }
  
  if (ret == -UNVALID_SETTING) {
    Serial.println("Recovered settings are not valid");
  } else {
    Serial.println("Recovered settings are valid");
  }
  return ret;
}

/**
 * @brief Save settings in EEPROM
 * 
 * save the settings and checks if the save is valid
 * and return if the save is valid or not
 * 
 * @param settings_ctx
 * 
 * @return 0 if the save is valid, -UNVALID_CHECK_SAVE if not
*/
static int save_settings_ctx(settings_context *settings_ctx)
{
  settings_context settings_ctx_check;
  int ret = 0;
  int address = 1;
  int i = 0;
  Serial.println("Saving settings");
  EEPROM.writeByte(address, settings_ctx->mode_jauge_smiley);
  address += sizeof(byte);
  EEPROM.writeUShort(address, settings_ctx->seuil_1);
  address += sizeof(unsigned short);
  EEPROM.writeUShort(address, settings_ctx->seuil_2);
  address += sizeof(unsigned short);
  EEPROM.writeUShort(address, settings_ctx->brightness);
  address += sizeof(unsigned short);
  EEPROM.writeULong(address, settings_ctx->sensitivity);
  address += sizeof(unsigned long);
  EEPROM.writeUShort(address, settings_ctx->connect_type);
  address += sizeof(unsigned short);
  for (i = 0; i < MAX_FIELD_SIZE; i++) {
    EEPROM.writeChar(address + (i * sizeof(char)), settings_ctx->wifi_ssid[i]);
  }
  address += i * sizeof(char);
  for (i = 0; i < MAX_FIELD_SIZE; i++) {
    EEPROM.writeChar(address + (i * sizeof(char)), settings_ctx->wifi_pass[i]);
  }
 
  EEPROM.commit();
  //verification
  Serial.println("Verifying save");
  address = 1;
  if (settings_ctx->mode_jauge_smiley != EEPROM.readByte(address)) {
    ret = -UNVALID_CHECK_SAVE;
  }
  
  address += sizeof(byte);
  if (settings_ctx->seuil_1 != EEPROM.readUShort(address)) {
    ret = -UNVALID_CHECK_SAVE;
  }
  
  address += sizeof(unsigned short);
  if (settings_ctx->seuil_2 != EEPROM.readUShort(address)) {
    ret = -UNVALID_CHECK_SAVE;
  }

  address += sizeof(unsigned short);
  if (settings_ctx->brightness != EEPROM.readUShort(address)) {
    ret = -UNVALID_CHECK_SAVE;
  }

  address += sizeof(unsigned short);
  if (settings_ctx->sensitivity != EEPROM.readULong(address)) {
    ret = -UNVALID_CHECK_SAVE;
  }
  address += sizeof(unsigned long);
  if (settings_ctx->connect_type != EEPROM.readUShort(address)) {
    ret = -UNVALID_CHECK_SAVE;
  }
  
  address += sizeof(unsigned short);
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    if (settings_ctx->wifi_ssid[i] != EEPROM.readChar(address + i * sizeof(char))) {
      ret = -UNVALID_CHECK_SAVE;
    }
  }
  
  address += MAX_FIELD_SIZE * sizeof(char);
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    if (settings_ctx->wifi_pass[i] != EEPROM.readChar(address + i * sizeof(char))) {
      ret = -UNVALID_CHECK_SAVE;
    }
  }
  if (ret == 0) {
    Serial.println("Save OK");
  } else {
    Serial.println("Save KO");
  }
  return ret;
}

/**
 * @brief initialize the connection to wifi network
 * 
 * this function try to connect to the wifi network provided by the user
 * if the connection fails, the hotspot is started with the default ssid and password
 * 
 * @param settings_ctx
 * 
 * @return wifi_state_enum : the state of the wifi connection
*/
static wifi_state_enum init_wifi_connect(settings_context *settings_ctx)
{
  wifi_state_enum ret;
  int i = 0;
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(settings_ctx->wifi_ssid);

  WiFi.begin(settings_ctx->wifi_ssid, settings_ctx->wifi_pass);
  WiFi.mode(WIFI_STA);
  for (i = 0; (i < 100) && (WiFi.status() != WL_CONNECTED); i++) {
      delay(500);
      Serial.print(".");
  }

  if (i > 99) {
    settings_ctx->connect_type = 0;
    settings_ctx->state_wifi = HOTSPOT_NOT_CONNECTED;
    Serial.println("connexion timeout !");
    ret = HOTSPOT_NOT_CONNECTED;
    for (int i = 0; i < MAX_FIELD_SIZE; i++) {
      settings_ctx->wifi_ssid[i] = ssid[i];
      settings_ctx->wifi_pass[i] = password[i];
    }
    save_settings_ctx(settings_ctx);
  } else {
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
    ret = WIFI_CONNECTED;
  }
  return ret;
}

/**
 * @brief function that init the wifi module to hotspot mode
 * 
 * the ssid and password are set from the settings_ctx
 * 
 * @param settings_ctx
*/
static void init_hotspot(settings_context *settings_ctx)
{
  WiFi.softAP(settings_ctx->wifi_ssid, settings_ctx->wifi_pass); 
  
  IPAddress gateway(192, 168, 4, 1);
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.mode(WIFI_AP);
  Serial.print("AP IP address: ");
  Serial.println(local_IP);
  server.begin();
  Serial.println("Server started");
  Serial.print("ssid : ");
  Serial.println(settings_ctx->wifi_ssid);   
  Serial.print("pass : ");
  Serial.println(settings_ctx->wifi_pass);
}

/**
 * @brief function that manage the web requests to/from the user
 * 
 * the function has to be called in the main loop, it is reading requests 
 * from the user's browser and send the web page where the settings of the device 
 * can be modified
 * 
 * @param settings_ctx
*/
static void web_management(settings_context *settings_ctx)
{
  String string_html;
  int mult = 0;
  WiFiClient client = server.available();   // listen for incoming clients
  int j;
  int last_res;
  uint8_t old_connect_type = settings_ctx->connect_type;

  if (client) {    // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    
    if (client.connected()) {

      if (client.available()) {             // if there's bytes to read from the client,
        currentLine = client.readString();             // read a byte, then
        Serial.print("DEBUG: ");
        Serial.println(currentLine);                    // print it out the serial monitor

        if (currentLine.startsWith("200") > 0) {
          currentLine = "";
          delay(100);
          client.readString();
          Serial.println("Client ack ok");
        } else if (currentLine.startsWith("GET") > 0) {
          currentLine = "";
          Serial.println("Client get received");
          page_hotspot_string(&string_html, *settings_ctx);
          client.println(string_html);
          Serial.println(client.readString());
        } else if (currentLine.startsWith("POST") > 0) {
          int index = currentLine.indexOf("\n\r\n");
          String line;
          for (int i = index; currentLine[i] != '\0'; i++){
              line += currentLine[i];
          }
          Serial.println("\nDEBUG : LINE: "+ line);

          //index = 0;
          //index = line.indexOf("input1=") + sizeof("input1=") - 1;
          //string_result = line.substring(index, line.indexOf("&", index));
          //string_result.toCharArray(settings_ctx->wifi_ssid, MAX_FIELD_SIZE);
          snprintf(settings_ctx->wifi_ssid,
                      line.indexOf("input2=") - (line.indexOf("input1=") + 6),
                      "%s",
                      line.substring(line.indexOf("input1=") + 7, (line.indexOf("input2=") - 1)));

         
          snprintf(settings_ctx->wifi_pass,
                      line.indexOf("seuilHaut=") - (line.indexOf("input2=") + 6),
                      "%s",
                      line.substring(line.indexOf("input2=") + 7, (line.indexOf("seuilHaut=") - 1)));

          mult = 1;
          settings_ctx->seuil_1 = 0;
          for (int i = line.indexOf("seuilBas=") - 2; i > line.indexOf("seuilHaut=") + 9; i--) {
            settings_ctx->seuil_1 += mult * (line.charAt(i) - 48);
            mult = mult * 10;
          }
          
          mult = 1;
          settings_ctx->seuil_2 = 0;
          for (int i = line.indexOf("sensibilite=") - 2; i > line.indexOf("seuilBas=") + 8; i--) {
            settings_ctx->seuil_2 += mult * (line.charAt(i) - 48);
            mult = mult * 10;
          }
          
          mult = 1;
          settings_ctx->sensitivity = 0;
          for (int i = line.indexOf("luminosite=") - 2; i > line.indexOf("sensibilite=") + 11; i--) {
            settings_ctx->sensitivity += mult * (line.charAt(i) - 48);
            mult = mult * 10;
          }
          settings_ctx->sensitivity /= 128;

          mult = 1;
          settings_ctx->brightness = 0;
          if ((line.indexOf("connect_type=") > 0) && (line.indexOf("jauge=") > 0)) {
            last_res = line.indexOf("jauge=") - 2;
          } else if (line.indexOf("connect_type=") > 0) {
            last_res = line.indexOf("connect_type=") - 2;
          } else if(line.indexOf("jauge=") > 0) {
            last_res = line.indexOf("jauge=") - 2;
          } else {
            last_res = line.length() - 1;
          }
          for (int i = last_res; i > line.indexOf("luminosite=") + 10; i--) {
            settings_ctx->brightness += mult * (line.charAt(i) - 48);
            mult = mult * 10;
          }

          if (line.indexOf("jauge=") > 0) {
            settings_ctx->mode_jauge_smiley = 1;
          } else {
            settings_ctx->mode_jauge_smiley = 0;
          }

          if (line.indexOf("connect_type=") > 0) {
            settings_ctx->connect_type = 1;
          } else {
            settings_ctx->connect_type = 0;
          }

          Serial.print("sizeof line = ");
          Serial.println(line.length());
          Serial.print("saved :\nssid : ");
          Serial.println(settings_ctx->wifi_ssid);
          Serial.print("pass : ");
          Serial.println(settings_ctx->wifi_pass);
          Serial.print("seuil_haut : ");
          Serial.println(settings_ctx->seuil_1);
          Serial.print("seuil_bas : ");
          Serial.println(settings_ctx->seuil_2);
          Serial.print("sensi : ");
          Serial.println(settings_ctx->sensitivity);
          Serial.print("brightness : ");
          Serial.println(settings_ctx->brightness);
          Serial.print("connect_type : ");
          Serial.println(settings_ctx->connect_type);
          Serial.print("jauge : ");
          Serial.println(settings_ctx->mode_jauge_smiley);

          for (j = 0; (j < MAX_SSID_LEN) && (settings_ctx->wifi_pass[j] != '\0'); j++) {}
          if ((settings_ctx->connect_type == 1) && (j >= 8)) {
            settings_ctx->connect_type = 1;
            if (old_connect_type == settings_ctx->connect_type) {
              settings_ctx->state_wifi = WIFI_CONNECTED;
            } else {
              settings_ctx->state_wifi = WIFI_NOT_CONNECTED;
            }
            Serial.println("Wifi selected");
          } else {
            settings_ctx->connect_type = 0;
            if (old_connect_type == settings_ctx->connect_type) {
              settings_ctx->state_wifi = HOTSPOT_CONNECTED;
            } else {
              settings_ctx->state_wifi = HOTSPOT_NOT_CONNECTED;
            }
            Serial.println("Password too short or wifi not selected");
          }
          while (client.available()) {
            //do nothing until the string of the request has been received
            client.read();
          }

          save_settings_ctx(settings_ctx);
          //after getting the post request, send the web page and wait for the ack: 
          page_hotspot_string(&string_html, *settings_ctx);
          client.println(string_html);
          currentLine = "";
        }
        
        
      
      } else {
        count++;
        if (count > 10000) {
          count = 0;
          Serial.println("Client unavailable");
        }
      }
     }

  } else {
    count++;
    if (count > 10000) {
      count = 0;
      Serial.println("Searching for client");
    }
  }
}

static void hotspot_not_connected(settings_context *settings_ctx, idle_client_server *wifi_idle_client_server)
{
  switch (*wifi_idle_client_server) {
    case CLIENT://case when the device is comming from wifi setup
      Serial.println("stoping previous server");
      server.end();
      WiFi.disconnect();
      *wifi_idle_client_server = HOTSPOT;
    case IDLE:
    case HOTSPOT:
      init_hotspot(settings_ctx);
      settings_ctx->state_wifi = HOTSPOT_CONNECTED;
      Serial.println("Going in hotspot connected");
      break;
    }
}

/**
 * @brief initialise the settings of the device
 * 
 * fill the settings_ctx with the default values (0)
 * and the wifi ssid/password with default ones (BAB_0/BAB_bpmp)
 * @param settings_ctx
*/
static void init_settings_ctx(settings_context *settings_ctx) {
  Serial.println("init settings ctx");
  settings_ctx->state_wifi = WIFI_NO_STATE;
  settings_ctx->seuil_1 = 70;
  settings_ctx->seuil_2 = 100;
  settings_ctx->sensitivity = 1;
  settings_ctx->brightness = 0;
  settings_ctx->mode_jauge_smiley = 0;
  
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    settings_ctx->wifi_ssid[i] = ssid[i];
    settings_ctx->wifi_pass[i] = password[i];
  }
}

/**
 * @brief read the eeprom de know if this is the first time the device is booting
 * 
 * @return int 1 if first boot, 0 otherwise
 */
static int is_first_boot() {
  int ret = 0;
  if (EEPROM.read(0) != 0x01) {
    ret = 1;
    EEPROM.writeByte(0, 0x01);
    Serial.println("First boot");
  }
  return ret;
}

void init_wifi_management(settings_context *settings_ctx, idle_client_server *wifi_idle_client_server)
{
  int loop = 1;
  int ret = 0;

  Serial.begin(115200);
  Serial.println();
  EEPROM.begin(EEPROM_SIZE);

  if (is_first_boot()) {
    init_settings_ctx(settings_ctx);
    save_settings_ctx(settings_ctx);
  } else {
    get_settings_ctx(settings_ctx);
  }
  if (settings_ctx->connect_type) {
    settings_ctx->state_wifi = WIFI_NOT_CONNECTED;
    Serial.println("Wifi selected");
  } else {
    settings_ctx->state_wifi = HOTSPOT_NOT_CONNECTED;
    Serial.println("Hotspot selected");
  }
  
  Serial.println(settings_ctx->state_wifi);
}

void wifi_management(settings_context *settings_ctx, idle_client_server *wifi_idle_client_server)
{
  //wifi state machine :
  switch (settings_ctx->state_wifi) {
    case WIFI_NO_STATE:
      Serial.println("State wifi not defined !\nSetting up wifi and going to hotspot not connected");
      settings_ctx->state_wifi = HOTSPOT_NOT_CONNECTED;
    break;
    
    case HOTSPOT_NOT_CONNECTED:
      Serial.println("Hotspot not connected");
      hotspot_not_connected(settings_ctx, wifi_idle_client_server);
    break;

    case HOTSPOT_CONNECTED:
      web_management(settings_ctx);
    break;      

    case WIFI_NOT_CONNECTED:
      settings_ctx->state_wifi = init_wifi_connect(settings_ctx);
    break;

    case WIFI_CONNECTED:
      if (WiFi.status() == WL_CONNECTED) {
        web_management(settings_ctx);
      } else {
        settings_ctx->state_wifi = WIFI_NOT_CONNECTED;
      }
    break;
  }
}