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
#include <math.h>

#define a 3.00786665 // coeficient a de la fonction a.e^bx
#define b 0.0781188 // coeficient b de la fonction a.e^bx


static const i2s_config_t i2s_config = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
     .sample_rate = 22050,
     .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
     .intr_alloc_flags = 0, // default interrupt priority
     .dma_buf_count = 8,
     .dma_buf_len = 64,
     .use_apll = false
};

// For Adafruit Huzzah Esp32

static const i2s_pin_config_t pin_config = {
    .bck_io_num = 26,                   // BCKL
    .ws_io_num = 25,                    // LRCL
    .data_out_num = I2S_PIN_NO_CHANGE,  // not used (only for speakers)
    .data_in_num = 33,                   // DOUT
};


class Microphone{
    private:
                
        size_t size;
        static const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number
        int32_t audio_buf[BUFLEN];

    public:
        int audio_value;
        float dB_audio;

        void setup_mic();
        void mic_get_val();
        float get_dB_value(int value);

};
