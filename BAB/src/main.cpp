

#include <FastLED.h>
#include "micros.h"
#include "matrice.cpp"


#define MIC_PIN 34 //mic
#define DATA_PIN 4 //led
#define BUTTON_PIN 21
#define VAL_MID 256
#define SIZE_TAB 16
#define SIZE_HALF_TAB 8
#define DELAY 300
#define LUM_MAX 50

bool state = 0;
uint8_t tab_mem[SIZE_TAB][SIZE_TAB];
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



uint8_t calc_dB_level(Microphone* micro){
  uint32_t buffer = 0;
  uint8_t buffer_size = 100;
  for(int i = 0; i != buffer_size; i++){
    micro->mic_get_val();
    buffer += micro->audio_value;
  }
  
  return micro->get_dB_value(buffer / buffer_size);
}

// Affichage des smiley en fonction volume actuel
// threshold Smiley 
// Happy: dB < 60 : 0 
// Close: dB < 100 : 1
// Angry: else   : 2
// Color RBG: 0 -> LUM_MAX :: 0 -> 120 (dB)


void display_smiley(uint8_t dB_val)
{
  // smiley is 0 if db_val-70 < 0 else 1 + int(dB_val / 100)
  uint8_t smiley = ((dB_val-70) < 0) ? 0 : 1 + (dB_val / 100);
  uint8_t r = dB_val * LUM_MAX / 120; // from 0 to LUM_MAX
  uint8_t g = LUM_MAX - dB_val * LUM_MAX / 120; // from LUM_MAX to 0  

  for(uint8_t i = 0; i != MID_NUM_LED; i++  ){
    bool led  = tab_eyes[smiley][i];
    leds[i] = CRGB(g * led, r * led, 0);
    led  = tab_mouth[smiley][i];
    leds[i + MID_NUM_LED] = CRGB(g * led, r * led, 0);

  }
  FastLED.show();  
}

//------------------------------------------------------------//


// remplissage de la matrice de led 
void fill_tab(int val_final)
{

  int buf_case;
  int buf_line[SIZE_TAB];

  //Serial.print("DEBUG: ");Serial.print(val_final);  
  for (int i = 0; i <  SIZE_TAB; i++) {
    for (int j = 0; j < SIZE_TAB; j++) {
      
      if (i) {
        buf_case = tab_mem[i][j];
        tab_mem[i][j] = buf_line[j];
        buf_line[j] = buf_case;
      } else {
        buf_line[j] = tab_mem[i][j];
        if (j < (val_final * SIZE_TAB) / 1024) {
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
        g = (LUM_MAX * (16 - j)) / SIZE_TAB;
      } else {
        r = 0;
        g = 0;
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
  }

  if (state){
    display_smiley(calc_dB_level(&micro));
  }else{
    micro.mic_get_val();
    fill_tab(micro.audio_value);
    display_gauge();
  }
  usleep(DELAY);

}