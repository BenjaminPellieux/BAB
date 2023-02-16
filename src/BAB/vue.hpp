
#include <FastLED.h>
#define DATA_PIN 4 //led
#define NUM_LEDS 256

#define VAL_MID 250
#define SIZE_TAB 16


enum DISPLAY{
  HIGH,
  GAUGE,
  SMILEY
};


class Vue{


  private:
    uint8_t tab_mem[NUM_LEDS][NUM_LEDS];
    CRGB leds[NUM_LEDS];
    int tab_dB_value[] = {30,60,90}; 
    int count_for_selected = 0;


  public:
    Vue(){

      FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);


    }
    // Affichage des smiley en fonction volume actuel

    //------------------------------------------------------------//

    // METHODE pour l'IHM
    
    void display_smiley(int db_value);
    void display_menu_gauge(uint8_t gauge_nbr, int state_nbr, uint8_t cursor_line, int selected_line, bool selected_bool, float val_gauge);
    void fill_tab(int dB_value);
    void display_amplitude();

};
