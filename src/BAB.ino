

#include <FastLED.h>
#include "micro.h"
#include "matrice.cpp"


#define MIC_PIN 34 //mic
#define DATA_PIN 4 //led
#define BUTTON_PIN 21
#define VAL_MID 250
#define PI 3,14
#define SIZE_TAB 16
#define POT_PIN 12

#define DISPLAY_GAUGE 2
#define DISPLAY_SMILEY 1
#define BUTTON_HIGH 0
#define SIZE_HALF_TAB 8


int delay_int = 100;
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

    for (int i = 0; i < NUM_LEDS / 2; i++) {
      leds[i] = CRGB(lum_max * tab_eyes_open[i], 0, 0);
      leds[i + NUM_LEDS / 2] = CRGB(lum_max * tab_mouth_happy[i], 0, 0);

    }
  } else if ((val_final > noise_thr) && (val_final < loud_thr)) {
    
    for (int i = 0; i < NUM_LEDS / 2; i++) {
      leds[i] = CRGB(lum_max * tab_eyes_close[i] / 2, lum_max * tab_eyes_close[i], 0);
      leds[i + NUM_LEDS / 2] = CRGB(lum_max * tab_mouth_ok[i] / 2, lum_max * tab_mouth_ok[i], 0);
    }
  } else { 

    for (int i = 0; i < NUM_LEDS / 2; i++) {
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


void display_menu_gauge(uint8_t gauge_nbr, int state_nbr, uint8_t cursor_line, int selected_line, bool selected_bool, float val_gauge) {
  count_for_selected++;
  int r = 0;
  int g = 0;
  if (count_for_selected > 314) {
    count_for_selected = 0;
  }
  for (int i = 0; i < gauge_nbr * 2 + state_nbr * 2; i++) {
    for (int j = 0; j < SIZE_TAB; j++) {
      if (selected_bool && (selected_line == i)) {
        if ((val_gauge >= j - 1) && (val_gauge <= j + 1)) {
          leds[(i * SIZE_TAB) + j] = CRGB(lum_max - (lum_max * abs(val_gauge - (SIZE_TAB + 0.5))),
                          lum_max * abs(val_gauge - (SIZE_TAB + 0.5)),0);          
        } else {
          leds[(i * SIZE_TAB) + j] = CRGB(lum_max, lum_max, 0);
        }
      } else if ((i == cursor_line) && (j == 0)) {
          leds[i * SIZE_TAB] = CRGB(lum_max, lum_max, lum_max);
      } else {
        leds[(i * SIZE_TAB) + j] = CRGB(0 , 0, lum_max);
      }      
    }
  }
  for (int i = (gauge_nbr * 2) + (state_nbr * 2); i < SIZE_TAB; i++) {
    for (int j = 0; j < SIZE_TAB; j++) {
      leds[(i * SIZE_TAB) + j] = CRGB(0, 0, 0);
    }
  }   
  for (int i = 1; i < (gauge_nbr * 2) + (state_nbr * 2); i += 2) {
    for (int j = 0; j < SIZE_TAB; j++) {
      leds[(i * SIZE_TAB) + j] = CRGB(0, 0, 0);
    }
  }   
  for (int i = (gauge_nbr * 2); i < SIZE_TAB; i++) {
    for (int j = 1; j < SIZE_TAB; j++) {
      leds[(i * SIZE_TAB) + j] = CRGB(0, 0, 0);
    }
  } 
  for (int j = noise_thr; j < loud_thr; j++) { 
    leds[(14 * SIZE_TAB) + j] = CRGB(0, lum_max, 0);
  }
  for (int j = 0; j < SIZE_TAB; j++) { //to replace buy a float if not working 
    r = sin(((j * 20) + count_for_selected) / 16) * lum_max;
    g = lum_max - (sin(((j * 20) + count_for_selected) / 16) * lum_max);
    leds[(15 * SIZE_TAB) + j] = CRGB(r, g, 0);
  }
  FastLED.show(); 
}
// menu de configuration
void menu()
{
  bool exit_menu = 0; // boolean sortie du menu
  int button_val = 0;  
  int selected_bool = 1;
  int cursor_line = 0;
  int gauge_nbr = 3;
  int state_nbr = 2;
  int selected_line = 0;
  float analog_val;
  while (!exit_menu) {
    button_val = digitalRead(BUTTON_PIN);
    
    if (!button_val) {
      selected_line = cursor_line * 2;
      selected_bool = !selected_bool;
      usleep(10000);
      uint8_t button_pressed = 0;
      while (!button_val) {
        button_val = digitalRead(BUTTON_PIN);
        button_pressed++;
        //Serial.println("Button pressed !");        
        usleep(1000);
      }

      if (selected_line > (gauge_nbr * 2) - 1) {
        state = (cursor_line - gauge_nbr) + 1;
        exit_menu = 1;
        
      }

      if (button_pressed > 100) {
        exit_menu = 1;
        //Serial.println("Menu exited");
      } 
    }
    
    pot_val = analogRead(POT_PIN);
    
    if (selected_bool) {
      analog_val = pot_val / 256;
    } else {
      cursor_line = (pot_val * (gauge_nbr + state_nbr)) / (256 * SIZE_TAB);       
    }
      
    if (cursor_line < 0) {
      cursor_line = 0;      
    }
    if (analog_val < 0) {
      analog_val = 0;
    }
    if (cursor_line > gauge_nbr + 1) {
      cursor_line = gauge_nbr + 1;
    }
    if (analog_val > SIZE_TAB) {
      analog_val = SIZE_TAB;
    }
    
    if (selected_bool) {
      switch(cursor_line) {
        case 1:
          delay_int = analog_val * 100;
          break;
        case 2:
          lum_max = analog_val * 10;
          break;          
        case 3:
          noise_thr = analog_val*10;
          break;
      }
    }
    display_menu_gauge(gauge_nbr, state_nbr, cursor_line * 2, selected_line, selected_bool, analog_val);
    usleep(delay_int);
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
  int button_val = digitalRead(BUTTON_PIN); //to replace with interrupt 

  if (!button_val) {
    state = BUTTON_HIGH;
    //Serial.println("Button pressed 1");
  }
  
  val_final = mic_get_val(); //get noise volume
  switch(state) {
    case BUTTON_HIGH:
      menu(); //get state value choosen by user
    break;

    case DISPLAY_SMILEY:
      display_smiley();
    break;

    case DISPLAY_GAUGE:
      fill_tab();
      display_gauge();
    break;

    default:
      //Serial.print("No state number :");
      //Serial.println(state);
    break;
  }
  usleep(delay_int);
}