/*
  Credits : Pierre BABIAN	pierrebabian4@gmail.com
  This is a script for wifi management on esp32
  
  - I used SimpleWiFiServer and WiFiAccessPoint from WiFi examples 	-
  - And libraries that it is using 									-
*/
#include <EEPROM.h>//https://github.com/espressif/arduino-esp32/tree/master/libraries/EEPROM
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#define EEPROM_SIZE 12
#define MAX_FIELD_SIZE	64

//errors :
#define UNVALID_SETTING	1
#define UNVALID_CHECK_SAVE	2

//addresses :
#define BRIGHTNESS_ADRESS	0
#define SENSIVITY_ADRESS	2
#define STATE_WIFI_ADRESS	6
#define SSID_ADRESS		8
#define PWD_ADRESS			MAX_FIELD_SIZE + SSID_ADRESS

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
  HOTSPOT_NOT_CONNECTED,
  HOTSPOT_CONNECTED,
  WIFI_NOT_CONNECTED,
  WIFI_CONNECTED
};

// Set these to your desired credentials.
const char *ssid = "BAB_0";
const char *password = "BAB_bpmp";

WiFiServer server(80);

//context (saved in eeprom): 
struct settings_context {
  uint16_t brightness; //0 - 255
  uint32_t sensitivity; //0 - 255
  //...
  wifi_state_enum state_wifi_on_boot;
  uint8_t connect_type; //hotspot, wifi
  char wifi_ssid[MAX_FIELD_SIZE];
  char wifi_pass[MAX_FIELD_SIZE];
  //wifi secu type ?
  //...  
};

int get_settings_ctx(settings_context *settings_ctx)
{
	int ret = 0;
	int address = BRIGHTNESS_ADRESS;
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
	/*
	address = SSID_ADRESS;
	for (int i = 0; i < MAX_FIELD_SIZE; i++) {
		settings_ctx->wifi_ssid[i] = EEPROM.readChar(address + i);
	}

	address = PWD_ADRESS;
	for (int i = 0; i < MAX_FIELD_SIZE; i++) {
		settings_ctx->wifi_pass[i] = EEPROM.readChar(address + i);
	}
 */
	return ret;
}



int save_settings_ctx(settings_context settings_ctx)
{
	int ret = 0;
	int address = BRIGHTNESS_ADRESS;
	EEPROM.writeUShort(address, settings_ctx.brightness);
	
	
	address = SENSIVITY_ADRESS;
	EEPROM.writeULong(address, settings_ctx.sensitivity);
	
	address = STATE_WIFI_ADRESS;
	EEPROM.writeUShort(address, settings_ctx.state_wifi_on_boot);
	/*
	address = SSID_ADRESS;
	for (int i = 0; i < MAX_FIELD_SIZE; i++) {
		EEPROM.writeChar(address + i, settings_ctx.wifi_ssid[i]);
	}

	address = PWD_ADRESS;
	for (int i = 0; i < MAX_FIELD_SIZE; i++) {
		EEPROM.writeChar(address + i, settings_ctx.wifi_pass[i]);
	}
	*/
	EEPROM.commit();
	
	//verification
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
	/*
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
 */
	return ret;
}

/*
struct settings_context {
  uint16_t brightness; //0 - 255
  uint32_t sensivity; //0 - 255

  uint16_t state_wifi_on_boot;
  uint8_t connect_type; //hotspot, wifi
  char wifi_ssid[64];
  char wifi_pass[64];
};
  uint8_t readByte(int address);
    int8_t readChar(int address);
    uint8_t readUChar(int address);
    int16_t readShort(int address);
    uint16_t readUShort(int address);
    int32_t readInt(int address);
    uint32_t readUInt(int address);
    int32_t readLong(int address);
    uint32_t readULong(int address);
    int64_t readLong64(int address);
    uint64_t readULong64(int address);
    

    size_t writeByte(int address, uint8_t value);
    size_t writeChar(int address, int8_t value);
    size_t writeUChar(int address, uint8_t value);
    size_t writeShort(int address, int16_t value);
    size_t writeUShort(int address, uint16_t value);
    size_t writeInt(int address, int32_t value);
    size_t writeUInt(int address, uint32_t value);
    size_t writeLong(int address, int32_t value);
    size_t writeULong(int address, uint32_t value);
    size_t writeLong64(int address, int64_t value);
    size_t writeULong64(int address, uint64_t value);
*/

int init_wifi_connect(settings_context *settings_ctx)
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

void init_hotspot()
{
   WiFi.softAP(ssid, password);
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
  WiFiClient client = server.available();   // listen for incoming clients
  Serial.println("Searching for client");
  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

			//TODO replace with the configuration page
            /* the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn ON the LED.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn OFF the LED.<br>");
			*/
			
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
		
		//TODO replace with settings reading
		//here the wifi connection to a lan network can be done (dont forget to change states)
		//contexts should be saved
        /* Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(LED_BUILTIN, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(LED_BUILTIN, LOW);                // GET /L turns the LED off
        }
		*/
		      save_settings_ctx(*settings_ctx);
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

void hotspot_connected(wifi_state_enum *state_wifi, settings_context *settings_ctx, uint16_t *wifi_idle_client_server)
{
		user_management_hotspot(state_wifi, settings_ctx);
}

void hotspot_not_connected(wifi_state_enum *state_wifi, uint16_t *wifi_idle_client_server)
{
	switch (*wifi_idle_client_server) {
	case CLIENT://case when the device is comming from wifi setup
		server.end();
		WiFi.disconnect();
		*wifi_idle_client_server = HOTSPOT;
  case IDLE:
	case HOTSPOT:
		init_hotspot();
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
  uint16_t wifi_idle_client_server = 0; //0 on idle, 1 on hotspot and 2 on server
  
  //TODO if first boot : default params
  //editing ctx and save it
  //end first boot


  while (loop) {
	//wifi state machine :
    switch (state_wifi) {
      case HOTSPOT_NOT_CONNECTED:
		  hotspot_not_connected(&state_wifi, &wifi_idle_client_server);
		  break;

      case HOTSPOT_CONNECTED:
		     (&state_wifi, &settings_ctx, &wifi_idle_client_server);
		  break;      

      case WIFI_NOT_CONNECTED:
		wifi_not_connected(&state_wifi, &settings_ctx, &wifi_idle_client_server);
		break;

      case WIFI_CONNECTED:
		wifi_connected(&state_wifi, &settings_ctx, &wifi_idle_client_server);
		break;

      default:
		Serial.println("State wifi not defined !\nGoing to hotspot not connected");
		state_wifi = HOTSPOT_NOT_CONNECTED;
		break;
    }
  }
}

//--------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------//

//TODO :
//first boot
//load contexts (eeprom)
//save contexts (eeprom)
//user managements -> web pages (using post requests for more security)
//be aware of the id/password overflow and code injections
