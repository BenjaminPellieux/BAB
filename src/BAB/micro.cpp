
#include "model.hpp"

// #if !(USING_DEFAULT_ARDUINO_LOOP_STACK_SIZE)
//   //uint16_t USER_CONFIG_ARDUINO_LOOP_STACK_SIZE = 16384;
//   uint16_t USER_CONFIG_ARDUINO_LOOP_STACK_SIZE = 8192;
// #endif


Microphone::Microphone(){
  pinMode(22, INPUT);
  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);   //install and start i2s driver
  REG_SET_BIT(  I2S_TIMING_REG(i2s_num),BIT(9));   /*  #include "soc/i2s_reg.h"   I2S_NUM -> 0 or 1*/
  REG_SET_BIT( I2S_CONF_REG(i2s_num), I2S_RX_MSB_SHIFT);
  i2s_set_pin( i2s_num, &pin_config);

}


void Microphone::get_val_mic(){
  int bytes_read = i2s_read(i2s_num, this->audio_buf, sizeof(this->audio_buf), &this->size, portMAX_DELAY);
  int32_t cleanBuf[BUFLEN / 2] {0};
  int cleanBufIdx = 0;
  for (int i = 0; i < BUFLEN; i++)
    {
    if (audio_buf[i] != 0)    // Exclude values from other channel
    {
      cleanBuf[cleanBufIdx] = this->audio_buf[i] >> 14;
      cleanBufIdx++;
    }
  }
  float meanval = 0;
  int volCount = 0;
  for (int i=0; i < BUFLEN / 2; i++){
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
  this->value_audio = maxsample - minsample;
  this->dB_audio = log(this->value_audio / a)/b;
}
    


