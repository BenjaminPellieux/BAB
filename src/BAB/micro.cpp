#define BUFLEN 256
#include <Arduino.h>
#include <driver/i2s.h>
#include "soc/i2s_reg.h"



// #if !(USING_DEFAULT_ARDUINO_LOOP_STACK_SIZE)
//   //uint16_t USER_CONFIG_ARDUINO_LOOP_STACK_SIZE = 16384;
//   uint16_t USER_CONFIG_ARDUINO_LOOP_STACK_SIZE = 8192;
// #endif


size_t size;
static const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number
static uint8_t var = 0;
int32_t audio_buf[BUFLEN];

static const i2s_config_t i2s_config = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
     .sample_rate = 22050,
     .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
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

void setup_mic() 
{ 
  pinMode(22, INPUT);
  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);   //install and start i2s driver
  REG_SET_BIT(  I2S_TIMING_REG(i2s_num),BIT(9));   /*  #include "soc/i2s_reg.h"   I2S_NUM -> 0 or 1*/
  REG_SET_BIT( I2S_CONF_REG(i2s_num), I2S_RX_MSB_SHIFT);
  i2s_set_pin(i2s_num, &pin_config);
}



// void show_audio(){

//   Serial.print("AUDIO: ");
//   for(int i = 0; i!= BUFLEN; i++){
//     Serial.print(audio_buf[i]);
//   }
//   Serial.println(" ");
// }

int mic_get_val() {
    int bytes_read = i2s_read(i2s_num, audio_buf, sizeof(audio_buf), &size, portMAX_DELAY);
    int32_t cleanBuf[BUFLEN / 2] {0};
    int cleanBufIdx = 0;

    //show_audio();
    for (int i = 0; i < BUFLEN; i++)
    {
      if (audio_buf[i] != 0)    // Exclude values from other channel
      {
          cleanBuf[cleanBufIdx] = audio_buf[i] >> 14;
          cleanBufIdx++;
      }
    }
    float meanval = 0;
    int volCount = 0;
    for (int i=0; i < BUFLEN / 2; i++) 
    {
         if (cleanBuf[i] != 0)
         {
          meanval += cleanBuf[i];
          volCount++;
         }
    }
    
    meanval /= volCount;
    Serial.println(cleanBuf[0]);
    //Serial.print("meanval: ");Serial.println(meanval);

    // subtract it from all sapmles to get a 'normalized' output
    for (int i=0; i< volCount; i++) 
    {
        cleanBuf[i] -= meanval;
    }

    // find the 'peak to peak' max
    float maxsample, minsample;
    minsample = 100000;
    maxsample = -100000;
    for (int i=0; i<volCount; i++) {
      minsample = _min(minsample, cleanBuf[i]);
      maxsample = _max(maxsample, cleanBuf[i]);
    }
    float test  = (maxsample - minsample);
    //Serial.print("min: ");Serial.print(minsample);Serial.print(",");Serial.print("max: ");Serial.println(maxsample);    

    return (maxsample - minsample);
}


