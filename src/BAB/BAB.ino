#include "model.hpp"


int delay_int = 100;
uint16_t lum_max = 10; 
float pot_val = 0;
int button_rise_bool = 0;
int noise_thr = 15000;
int state = 0;

//Vue matrice_led = new Vue();
//Microphone microphone = new Microphone();

Vue matrice_led;
Microphone microphone;


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
    matrice_led.display_menu_gauge(gauge_nbr, state_nbr, cursor_line * 2, selected_line, selected_bool, analog_val,lum_max, noise_thr);
    usleep(delay_int);
  }
}
// fonction d'affchage des jauges 

//------------------------------------------------------------//

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(MIC_PIN, INPUT);
  pinMode(POT_PIN, INPUT);
  Serial.begin(9600);
  
}

//------------------------------------------------------------//

//------------------------------------------------------------//

void loop()
{
  int button_val = digitalRead(BUTTON_PIN); //to replace with interrupt 

  if (!button_val) {
    state = LEDISPLAY::MENU ;
    //Serial.println("Button pressed 1");
  }
  
  microphone.get_val_mic(); //get noise volume

  switch(state) {
    case LEDISPLAY::MENU:
      Serial.print("MENU::");
      Serial.println(state);
      menu(); //get state value choosen by user
    break;

    case LEDISPLAY::SMILEY:
      Serial.print("SMILEY::");
      Serial.println(state);
      matrice_led.display_smiley(microphone.dB_audio,lum_max);
    break;
    
    case LEDISPLAY::GAUGE:
      Serial.print("GAUGE::");
      Serial.println(state);
      matrice_led.fill_tab(microphone.value_audio);
      matrice_led.display_amplitude(lum_max);
    break;
  }
  usleep(delay_int);
}