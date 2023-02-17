
#ifndef Model
#define Model

#include <FastLED.h>
#include <Arduino.h>
#include <driver/i2s.h>
#include "soc/i2s_reg.h"
#include <math.h>
#include <stdint.h>


#define MIC_PIN 34 //mic
#define BUTTON_PIN 21
#define PI 3,14
#define POT_PIN 12
#define DATA_PIN 4 //Pin de controle des LEDS
#define NUM_LEDS 256 // Nombre de LEDS
#define VAL_MID 250 // 
#define SIZE_TAB 16 // Nombre de les par Ligne / Colone
#define BUFLEN 256 // Buffer for audio signial
#define a 3.00786665 // coeficient a de la fonction a.e^bx
#define b 0.0781188 // coeficient b de la fonction a.e^bx
#define SIZE_HALF_TAB 8
#define NB_PALIER 3// nombre de palier pour le smiley



static const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number
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
      
static constexpr i2s_pin_config_t pin_config = {
    .bck_io_num = 26,                   // BCKL
    .ws_io_num = 25,                    // LRCL
    .data_out_num = I2S_PIN_NO_CHANGE,  // not used (only for speakers)
    .data_in_num = 33,                   // DOUT
};


enum LEDISPLAY{
  MENU,
  GAUGE,
  SMILEY
};

class Microphone{
  private:
  
    size_t size;
    int32_t audio_buf[BUFLEN];

  public:

    int value_audio;
    float dB_audio;

    Microphone();
    void get_val_mic();
};


class Vue{

  private:
    uint8_t tab_mem[NUM_LEDS][NUM_LEDS];
    CRGB leds[NUM_LEDS];
    int tab_dB_value[NB_PALIER] = {30,60,90}; 
    int count_for_selected = 0;

    


  public:

    // Affichage des smiley en fonction volume actuel

    //------------------------------------------------------------//

    // METHODE pour l'IHM
    Vue();
    void display_smiley(int db_value,uint16_t lum_max);
    void display_menu_gauge(uint8_t gauge_nbr, int state_nbr, uint8_t cursor_line, int selected_line, bool selected_bool, float val_gauge, uint16_t lum_max, int noise_thr);
    void fill_tab(int dB_value);
    void display_amplitude(uint16_t lum_max);

};

#endif