#include "wifi.h"
#include "web.cpp"




int get_settings_ctx(settings_context *settings_ctx)
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



int save_settings_ctx(settings_context settings_ctx)
{
  int ret = 0;
  int address = 1;
  int i = 0;
  Serial.println("Saving settings");
  EEPROM.writeByte(address, settings_ctx.mode_jauge_smiley);
  address += sizeof(byte);
  EEPROM.writeUShort(address, settings_ctx.seuil_1);
  address += sizeof(unsigned short);
  EEPROM.writeUShort(address, settings_ctx.seuil_2);
  address += sizeof(unsigned short);
  EEPROM.writeUShort(address, settings_ctx.brightness);
  address += sizeof(unsigned short);
  EEPROM.writeULong(address, settings_ctx.sensitivity);
  address += sizeof(unsigned long);
  EEPROM.writeUShort(address, settings_ctx.connect_type);
  address += sizeof(unsigned short);
  for (i = 0; i < MAX_FIELD_SIZE; i++) {
    EEPROM.writeChar(address + (i * sizeof(char)), settings_ctx.wifi_ssid[i]);
  }
  address += i * sizeof(char);
  for (i = 0; i < MAX_FIELD_SIZE; i++) {
    EEPROM.writeChar(address + (i * sizeof(char)), settings_ctx.wifi_pass[i]);
  }
 
  EEPROM.commit();
  //verification
  Serial.println("Verifying save");
  address = 1;
  settings_ctx.mode_jauge_smiley = EEPROM.readByte(address);
  if ((settings_ctx.mode_jauge_smiley > 255) || (settings_ctx.mode_jauge_smiley < 0)) {
    ret = -UNVALID_SETTING;
  }
  
  address += sizeof(byte);
  settings_ctx.seuil_1 = EEPROM.readUShort(address);
  if ((settings_ctx.seuil_1 > 255) || (settings_ctx.seuil_1 < 0)) {
    ret = -UNVALID_CHECK_SAVE;
  }
  
  address += sizeof(unsigned short);
  settings_ctx.seuil_2 = EEPROM.readUShort(address);
  if ((settings_ctx.seuil_2 > 255) || (settings_ctx.seuil_2 < 0)) {
    ret = -UNVALID_CHECK_SAVE;
  }

  address += sizeof(unsigned short);
  if (settings_ctx.brightness != EEPROM.readUShort(address)) {
    ret = -UNVALID_CHECK_SAVE;
  }

  address += sizeof(unsigned short);
  if (settings_ctx.sensitivity != EEPROM.readULong(address)) {
    ret = -UNVALID_CHECK_SAVE;
  }
  address += sizeof(unsigned long);
  if (settings_ctx.connect_type != EEPROM.readUShort(address)) {
    ret = -UNVALID_CHECK_SAVE;
  }
  
  address += sizeof(unsigned short);
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    if (settings_ctx.wifi_ssid[i] != EEPROM.readChar(address + i * sizeof(char))) {
      ret = -UNVALID_CHECK_SAVE;
    }
  }
  
  address += MAX_FIELD_SIZE * sizeof(char);
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    if (settings_ctx.wifi_pass[i] != EEPROM.readChar(address + i * sizeof(char))) {
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

wifi_state_enum init_wifi_connect(settings_context *settings_ctx)
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
    settings_ctx->state_wifi = wifi_state_enum::HOTSPOT_NOT_CONNECTED;
    Serial.println("connexion timeout !");
    ret = wifi_state_enum::HOTSPOT_NOT_CONNECTED;
    for (int i = 0; i < MAX_FIELD_SIZE; i++) {
      settings_ctx->wifi_ssid[i] = ssid[i];
      settings_ctx->wifi_pass[i] = password[i];
    }
    save_settings_ctx(*settings_ctx);
  } else {
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
    ret = wifi_state_enum::WIFI_CONNECTED;
  }
  return ret;
}

void init_hotspot(settings_context *settings_ctx)
{

   //WiFi.softAP(settings_ctx->wifi_ssid, settings_ctx->wifi_pass);
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

void user_management(settings_context *settings_ctx)
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
        //Serial.println(currentLine);
        //Serial.println("DEBUG form : " + server.args("input2"))
        //Serial.println("DEBUG 1.3");

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
              settings_ctx->state_wifi = wifi_state_enum::WIFI_CONNECTED;
            } else {
              settings_ctx->state_wifi = wifi_state_enum::WIFI_NOT_CONNECTED;
            }
            Serial.println("Wifi selected");
          } else {
            settings_ctx->connect_type = 0;
            if (old_connect_type == settings_ctx->connect_type) {
              settings_ctx->state_wifi = wifi_state_enum::HOTSPOT_CONNECTED;
            } else {
              settings_ctx->state_wifi = wifi_state_enum::HOTSPOT_NOT_CONNECTED;
            }
            Serial.println("Password too short or wifi not selected");
          }
          while (client.available()) {
            //do nothing until the string of the request has been received
            client.read();
          }

          save_settings_ctx(*settings_ctx);
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
    //close the connection:
    //client.stop();
    //Serial.println("Client Disconnected.");
  } else {
    count++;
    if (count > 10000) {
      count = 0;
      Serial.println("Searching for client");
    }
  }
}
void hotspot_connected(settings_context *settings_ctx, uint16_t *wifi_idle_client_server)
{
    user_management(settings_ctx);
}

void hotspot_not_connected(settings_context *settings_ctx, uint16_t *wifi_idle_client_server)
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
    settings_ctx->state_wifi = wifi_state_enum::HOTSPOT_CONNECTED;
    Serial.println("Going in hotspot connected");
    break;
  }
}

void wifi_connected(settings_context *settings_ctx, uint16_t *wifi_idle_client_server)
{
  if (WiFi.status() == WL_CONNECTED) {
    user_management(settings_ctx);
  } else {
    settings_ctx->state_wifi = wifi_state_enum::WIFI_NOT_CONNECTED;
  }
}


void init_settings_ctx(settings_context *settings_ctx) {
  Serial.println("init settings ctx");
  settings_ctx->state_wifi = wifi_state_enum::WIFI_NO_STATE;
  settings_ctx->seuil_1 = 0;
  settings_ctx->seuil_2 = 0;
  settings_ctx->sensitivity = 0;
  settings_ctx->brightness = 0;
  settings_ctx->mode_jauge_smiley = 0;
  for (int i = 0; i < MAX_FIELD_SIZE; i++) {
    settings_ctx->wifi_ssid[i] = ssid[i];
    settings_ctx->wifi_pass[i] = password[i];
  }
}



//--------------------------------------------------------------------------------------//
//----------------------------------------MAIN------------------------------------------//
//--------------------------------------------------------------------------------------//


//--------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------//

//TODO :
//first boot
//get ssid and pasword from post response
//user managements -> web pages (using post requests for more security)
//be aware of the id/password overflow and code injections