#include "micros.h"


void Microphone::setup_mic(){
  pinMode(22, INPUT);
  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);   //install and start i2s driver
  REG_SET_BIT(  I2S_TIMING_REG(i2s_num),BIT(9));   /*  #include "soc/i2s_reg.h"   I2S_NUM -> 0 or 1*/
  REG_SET_BIT( I2S_CONF_REG(i2s_num), I2S_RX_MSB_SHIFT);
  i2s_set_pin(i2s_num, &pin_config);
}

uint32_t Microphone::mic_get_val() {
    size_t size;
    int32_t audio_buf[BUFLEN];
    int32_t cleanBuf[BUFLEN] {0};
    i2s_read(i2s_num, audio_buf, sizeof(audio_buf), &size, portMAX_DELAY);

    int cleanBufIdx = 0;
    int volCount = 0;
    float meanval = 0;

    for (int i = 0; i < BUFLEN; i++)
    {
      if (audio_buf[i] != 0)    // Exclude values from other channel
      {
        cleanBuf[cleanBufIdx] = audio_buf[i] >> 14;
          if (cleanBuf[cleanBufIdx] != 0){
            meanval += cleanBuf[i];
            volCount++;
          }
        cleanBufIdx++;
      }
    }
    meanval /= volCount;

    // subtract it from all sapmles to get a 'normalized' output
    for (int i=0; i < volCount; i++) 
    {
      cleanBuf[i] -= meanval;
    }

    // find the 'peak to peak' max
    float maxsample, minsample;
    minsample = 100000; maxsample = -100000;
    
    for (int i=0; i<volCount; i++) {
      minsample = _min(minsample, cleanBuf[i]); maxsample = _max(maxsample, cleanBuf[i]);
    }
    return (maxsample - minsample);
}

uint8_t Microphone::get_dB_value(int value){
  return log(value / CA)/ CB;
}