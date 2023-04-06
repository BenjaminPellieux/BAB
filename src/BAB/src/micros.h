/** 
 * ESP32 I2S VU Meter Example: 'Esp32_I2S_SPH0645_Microphone_Volume'
 * 
 * Used Board: Adafruit HUZZAH32 – ESP32 Feather Board
 * 
 * Gets the volume of surrounding noise using Adafruit I2S Microphone (SPH0645)
 * 
 * This example is based on the code of:
 * https://www.esp32.com/viewtopic.php?t=4997
 * and the "VU Meter Demo" Example in the 
 * Adafruit I2S MEMS Breakout board.
 * 
 * Some tricks to make the SPH0645 work properly on the ESP32
 * are explained in the above link and in this video:
 * https://www.youtube.com/watch?v=3g7l5bm7fZ8
 * 
 * @author RoSchmi
 */


// If default stack size of 8192 byte is not enough for your application.
// --> configure stack size dynamically from code to 16384
// https://community.platformio.org/t/esp32-stack-configuration-reloaded/20994/4
// Patch: Replace C:\Users\thisUser\.platformio\packages\framework-arduinoespressif32\cores\esp32\main.cpp
// with the file 'main.cpp' from folder 'patches' of this repository, then use the following code to configure stack size
// comment the following 4 lines if you do not want to use the patch




#define BUFLEN 256
#include <Arduino.h>
#include <driver/i2s.h>
#include "soc/i2s_reg.h"


void setup_mic();
int mic_get_val();
//void show_audio();
