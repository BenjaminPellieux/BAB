

#include <FastLED.h>
#include "micros.h"
#include "matrice.cpp"


#define MIC_PIN 34 //mic
#define DATA_PIN 4 //led
#define BUTTON_PIN 21
#define VAL_MID 256
#define SIZE_TAB 16
#define SIZE_HALF_TAB 8
#define DELAY 100
#define LUM_MAX 50
#define dB_MAX 120

uint16_t delay_val = DELAY;
bool state = 0;
bool tab_mem[SIZE_TAB][SIZE_TAB];
CRGB leds[NUM_LEDS];
Microphone micro;

//------------------------------------------------------------//

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(MIC_PIN, INPUT);
  Serial.begin(9600);
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
  micro.setup_mic();
  //attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), interrupt_button, RISING);
}

//------------------------------------------------------------//



/*
@ Function: calc_dB_median
This function calculates the avarage of a sample of a size pass as a parameter
@ Parameter:
- Microphone: to acces to the microphone methode
- uint8_t: the size of the sample
@ Author: Benjamin PELLIEUX
@ Date:  15/04/23
@ State:  Done 
*/
static uint8_t calc_dB_average(Microphone* micro,uint8_t sample_size){
  uint32_t sample = 0;
  for(int i = 0; i != sample_size; i++){
    micro->mic_get_val();
    sample += micro->audio_value;
  }
  return micro->get_dB_value(sample / sample_size);
}

/*
@ Function: calc_dB_median
This function calculates the median of a sample of a size pass as a parameter
@ Parameter:
- Microphone: to acces to the microphone methode
- uint8_t: the size of the sample
@ Author: Benjamin PELLIEUX
@ Date:  15/04/23
@ State:  Done 
*/

static uint8_t calc_dB_median(Microphone* micro,uint8_t sample_size){
  uint16_t *sample = (uint16_t*) malloc(sizeof(uint16_t) * sample_size);
  if (!sample){ // Check of thin provisioning exit if failure
    EXIT_FAILURE; 
  }
  bool flag;
  uint16_t tmp;
  for(int i = 0; i != sample_size; i++){// Collect all samples in a tab 
    micro->mic_get_val();
    sample[i] = micro->audio_value;
  }
  
  do{// short all samples using quick short as the sample size is small
    flag = 0;
    for(uint8_t k = 0; k != sample_size; k++){
      if (sample[k] > sample[k + 1]){
        tmp = sample[k + 1];
        sample[k + 1] = sample[k];
        sample[k] = tmp;
        flag = 1;
      }
    }

  }while(flag);
  uint8_t dB_value = micro->get_dB_value(sample[(int) (sample_size / 2)] / sample_size);
  free(sample); // Free the memory of the sample tab
  return dB_value;
}
/*
Display of smiley faces based on current volume
threshold Smiley 
Happy: dB < 70 : 0 
Close: dB < 100 : 1
Angry: else : 2
Color RBG: 0 -> LUM_MAX :: 
           0 -> 120 (dB)

@ Author: Pierre BABIAN & Benjamin PELLIEUX
@ Date:  15/04/23
@ State:  Done 
*/
void display_smiley(uint8_t dB_val)
{
  // smiley is 0 if db_val-70 < 0 else 1 + int(dB_val / 100)
  uint8_t smiley = ((dB_val-70) < 0) ? 0 : 1 + (dB_val / 100);
  uint8_t r = dB_val * LUM_MAX / dB_MAX; // from 0 to LUM_MAX
  uint8_t g = LUM_MAX - dB_val * LUM_MAX / dB_MAX; // from LUM_MAX to 0  
  bool led;

  for(uint8_t i = 0; i != MID_NUM_LED; i++ ){
    led  = tab_eyes[smiley][i];
    leds[i] = CRGB(g * led, r * led, 0);
    led  = tab_mouth[smiley][i];
    leds[i + MID_NUM_LED] = CRGB(g * led, r * led, 0);

  }
  FastLED.show();  
}

//------------------------------------------------------------//


// remplissage de la matrice de led 
void fill_tab(uint8_t dB_val)
{
  bool buf_case;
  bool buf_line[SIZE_TAB] = {0};
  
  for (int i = 0; i !=  SIZE_TAB; i++) {
    for (int j = 0; j != SIZE_TAB; j++) {
      
      if (i) {
        buf_case = tab_mem[i][j];
        tab_mem[i][j] = buf_line[j];
        buf_line[j] = buf_case;
      } else {
        buf_line[j] = tab_mem[i][j];
        if (j < (dB_val * SIZE_TAB) / dB_MAX) {
          tab_mem[i][j] = 1;
        } else {
          tab_mem[i][j] = 0;
        }
      }
    }
  }
}  



// fonction d'affchage des jauges 
void display_gauge()
{
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t used_j = 0;
  for (uint8_t i = 0; i < SIZE_TAB; i++) {
    for (uint8_t j = 0; j < SIZE_TAB; j++) {
      if (i % 2) {
        used_j = SIZE_TAB - j - 1;
      } else {
        used_j = j;
      }
      if (tab_mem[i][used_j]) {
        r = (LUM_MAX * j) / SIZE_TAB;
        g = (LUM_MAX * (SIZE_TAB - j)) / SIZE_TAB;
      } else {
        r = 0; g = 0;
      }
      
      leds[(i * SIZE_TAB) + j] = CRGB((r * (i % 2)) + (g * !(i % 2)),
                                      (r * !(i % 2)) + (g * (i % 2)),
                                       0);
    }
  }
  FastLED.show(); 
}

//------------------------------------------------------------//

void loop()
{
  if (!digitalRead(BUTTON_PIN)) {
    state = !state;
    usleep(delay_val);
  }
  //Serial.print("DEBUG STATE"  );Serial.println(state);
  if (state){
    display_smiley(calc_dB_average(&micro,50));
  }else{
    fill_tab(calc_dB_median(&micro,5));
    display_gauge();
  }
  usleep(delay_val);

}