
#include "micros.h"


// #if !(USING_DEFAULT_ARDUINO_LOOP_STACK_SIZE)
//   //uint16_t USER_CONFIG_ARDUINO_LOOP_STACK_SIZE = 16384;
//   uint16_t USER_CONFIG_ARDUINO_LOOP_STACK_SIZE = 8192;
// #endif


// void show_audio(){

//   Serial.print("AUDIO: ");
//   for(int i = 0; i!= BUFLEN; i++){
//     Serial.print(audio_buf[i]);
//   }
//   Serial.println(" ");
// }





void Microphone::setup_mic(){
  pinMode(22, INPUT);
  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);   //install and start i2s driver
  REG_SET_BIT(  I2S_TIMING_REG(i2s_num),BIT(9));   /*  #include "soc/i2s_reg.h"   I2S_NUM -> 0 or 1*/
  REG_SET_BIT( I2S_CONF_REG(i2s_num), I2S_RX_MSB_SHIFT);
  i2s_set_pin(i2s_num, &pin_config);
}

void Microphone::mic_get_val() {
    int bytes_read = i2s_read(i2s_num, audio_buf, sizeof(audio_buf), &size, portMAX_DELAY);
    int32_t cleanBuf[BUFLEN / 2] {0};
    int cleanBufIdx = 0;

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
    audio_value = (maxsample - minsample);
}

float Microphone::get_dB_value(){
  return log(audio_value / a)/b;
}