

#include <FastLED.h>
#include "micros.h"
#include "matrice.cpp"


#define MIC_PIN 34 //mic
#define DATA_PIN 4 //led
#define BUTTON_PIN 21
#define VAL_MID 256
#define SIZE_TAB 16
#define POT_PIN 12

#define DISPLAY_GAUGE 2
#define DISPLAY_SMILEY 1
#define BUTTON_HIGH 0
#define SIZE_HALF_TAB 8

int delay_int = 300;
uint16_t lum_max = 10;
uint16_t min_mic = 1024;
uint8_t button_state = 0; 
uint8_t first_run = 1;
uint8_t button_old = 0;
float pot_val = 0;
int val_final = 100;
int button_rise_bool = 0;
int noise_thr = 15000;
int state = 0;
int loud_thr = 200;
int count_for_selected = 0;





uint8_t tab_mem[NUM_LEDS][NUM_LEDS];
CRGB leds[NUM_LEDS];

//------------------------------------------------------------//

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(MIC_PIN, INPUT);
  pinMode(POT_PIN, INPUT);
  Serial.begin(9600);
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
  setup_mic();
  //attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), interrupt_button, RISING);
}

//------------------------------------------------------------//


// Affichage des smiley en fonction volume actuel
void display_smiley()
{
  if (val_final < noise_thr) {

    for (int i = 0; i < NUM_LEDS / 2; i++){
      leds[i] = CRGB(lum_max * tab_eyes_open[i], 0, 0);
      leds[i + NUM_LEDS / 2] = CRGB(lum_max * tab_mouth_happy[i], 0, 0);

    }
  } else if ((val_final > noise_thr) && (val_final < loud_thr)) {
    
    for (int i = 0; i < NUM_LEDS / 2; i++){
      leds[i] = CRGB(lum_max * tab_eyes_close[i] / 2, lum_max * tab_eyes_close[i], 0);
      leds[i + NUM_LEDS / 2] = CRGB(lum_max * tab_mouth_ok[i] / 2, lum_max * tab_mouth_ok[i], 0);
    }
  } else { 

    for (int i = 0; i < NUM_LEDS / 2; i++){
      leds[i] = CRGB(0, lum_max * tab_eyes_x[i], 0);
      leds[i + NUM_LEDS / 2] = CRGB(0, lum_max * tab_mouth_bad[i], 0);
    }

  }
  FastLED.show();  
  usleep(10000);  
}

//------------------------------------------------------------//


// remplissage de la matrice de led 
void fill_tab()
{

  int buf_case;
  int buf_line[SIZE_TAB];
  
  // Echele de valeur de val_final
  // 30 bd  20/ 40 
  // 50 dB 100/ 200
  // 60 dB 200/ 400
  // 70 dB 500/ 1000
  // 80 dB 1000/ 2000
  // 90 dB 2000/ 5000
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
          //Serial.println((val_final * SIZE_TAB) / 1024);
          tab_mem[i][j] = 1;
        } else {
          tab_mem[i][j] = 0;
        }
      }
    }
  }
}  

//------------------------------------------------------------//
// affichage du menu de parametrage
// Parametre: //////////////////////:::
// uint8_t gauge_nbr:  nombre de gauge
// int state:  etat de la gauge
// uint8_t cursor_line: position du cursor 
// int selected_line:  numero de la gauge selectionner
// bool selected_bool : selectioner ou en attente
// float val_gauge: valeur du potentiometre


// menu de configuration

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
        r = (lum_max * j) / SIZE_TAB;
        g = (lum_max * (16 - j)) / SIZE_TAB;
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
   //to replace with interrupt 

  if (!digitalRead(BUTTON_PIN)) {
    state = !state;
    //Serial.println("Button pressed 1");
  }
  
  val_final = mic_get_val(); //get noise volume
  if (state){
    display_smiley();
  }else{
    fill_tab();
    display_gauge();
  }
  usleep(delay_int);

}