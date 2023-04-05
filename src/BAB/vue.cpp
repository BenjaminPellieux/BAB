
#include "model.hpp"
#include "matrice.cpp"


Vue::Vue() {

  FastLED.addLeds<WS2811, DATA_PIN, RGB>(this->leds, NUM_LEDS); 
}

Vue::~Vue() {

  Serial.println("Good Night!");

}



void Vue::display_menu_gauge(uint8_t gauge_nbr, int state_nbr, uint8_t cursor_line, int selected_line, bool selected_bool, float val_gauge, uint16_t lum_max, int noise_thr) {
  this->count_for_selected++;
  int r = 0;
  int g = 0;
  if (count_for_selected > 314) {
    count_for_selected = 0;
  }
  for (int i = 0; i < gauge_nbr * 2 + state_nbr * 2; i++) {
    for (int j = 0; j < SIZE_TAB; j++) {
      if (selected_bool && (selected_line == i)) {
        if ((val_gauge >= j - 1) && (val_gauge <= j + 1)) {
          this->leds[(i * SIZE_TAB) + j] = CRGB(lum_max - (lum_max * abs(val_gauge - (SIZE_TAB + 0.5))),
                                                lum_max * abs(val_gauge - (SIZE_TAB + 0.5)), 0);
        } else {
          this->leds[(i * SIZE_TAB) + j] = CRGB(lum_max, lum_max, 0);
        }
      } else if ((i == cursor_line) && (j == 0)) {
        this->leds[i * SIZE_TAB] = CRGB(lum_max, lum_max, lum_max);
      } else {
        this->leds[(i * SIZE_TAB) + j] = CRGB(0, 0, lum_max);
      }
    }
  }
  for (int i = (gauge_nbr * 2) + (state_nbr * 2); i < SIZE_TAB; i++) {
    for (int j = 0; j < SIZE_TAB; j++) {
      this->leds[(i * SIZE_TAB) + j] = CRGB(0, 0, 0);
    }
  }
  for (int i = 1; i < (gauge_nbr * 2) + (state_nbr * 2); i += 2) {
    for (int j = 0; j < SIZE_TAB; j++) {
      this->leds[(i * SIZE_TAB) + j] = CRGB(0, 0, 0);
    }
  }
  for (int i = (gauge_nbr * 2); i < SIZE_TAB; i++) {
    for (int j = 1; j < SIZE_TAB; j++) {
      this->leds[(i * SIZE_TAB) + j] = CRGB(0, 0, 0);
    }
  }
  for (int j = noise_thr; j < 200; j++) {
    this->leds[(14 * SIZE_TAB) + j] = CRGB(0, lum_max, 0);
  }
  for (int j = 0; j < SIZE_TAB; j++) {  //to replace buy a float if not working
    r = sin(((j * 20) + this->count_for_selected) / 16) * lum_max;
    g = lum_max - (sin(((j * 20) + this->count_for_selected) / 16) * lum_max);
    this->leds[(15 * SIZE_TAB) + j] = CRGB(r, g, 0);
  }
  FastLED.show();
}


void Vue::display_amplitude(uint16_t lum_max) {
  Serial.println("Display Amplitude");
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t used_j = 0;
  for (uint8_t i = 0; i < SIZE_TAB; i++) {
    for (uint8_t j = 0; j < SIZE_TAB; j++) {
      if (i % 2) {
        used_j = SIZE_TAB - j - 1;
      }else {
        used_j = j;
      }
      //Serial.print("DEBUG: pos:");Serial.print(i);Serial.print("::");Serial.print(used_j);Serial.print("Value: ");Serial.println(this->tab_mem[i][used_j]);
      if (this->tab_mem[i][used_j]) {
        r = (lum_max * j) / SIZE_TAB;
        g = (lum_max * (16 - j)) / SIZE_TAB;
      }else {
        r = 0;
        g = 0;
      }
      Serial.print("DEBUG: LED1::");Serial.println((i * SIZE_TAB) + j);
      this->leds[(i * SIZE_TAB) + j] = CRGB((r * (i % 2)) + (g * !(i % 2)),
                                           (r * !(i % 2)) + (g * (i % 2)),
                                            0);
      
      
      //this->leds[(i * SIZE_TAB) + j] = CRGB(r , g, 0);
      //Serial.print("DEBUG: LED2::");Serial.println(this->leds[(i * SIZE_TAB) + j]);
    }
  }
  Serial.println("DEBUG: FastLED.show()");
  FastLED.show();
}


// remplissage de la matrice de led
void Vue::fill_tab(int value_audio) {
  Serial.println("FILL TAB");
  int buf_case;
  int buf_line[SIZE_TAB];

  for (int i = 0; i < SIZE_TAB; i++) {
    for (int j = 0; j < SIZE_TAB; j++) {

      if (i) {
        
        buf_case = this->tab_mem[i][j];
        this->tab_mem[i][j] = buf_line[j];
        buf_line[j] = buf_case;
      } else {
        
        buf_line[j] = this->tab_mem[i][j];
        //Serial.print("DEBUG: j: " );Serial.print(j);Serial.print(" Value: ");Serial.println(value_audio * SIZE_TAB / 1024);
        if (j < (value_audio * SIZE_TAB / 1024)) {

          this->tab_mem[i][j] = 1;
        } else {
          this->tab_mem[i][j] = 0;
        }
      }
    }
  }
}


void Vue::display_smiley(float dB_value, uint16_t lum_max) {
  Serial.print("Display audio::  ");Serial.println(dB_value);
  if (dB_value < this->tab_dB_value[0]) {

    for (int i = 0; i < NUM_LEDS / 2; i++) {
      this->leds[i] = CRGB(lum_max * tab_eyes_open[i], 0, 0);
      this->leds[i + NUM_LEDS / 2] = CRGB(lum_max * tab_mouth_happy[i], 0, 0);
    }
  } else if ((dB_value > this->tab_dB_value[1]) && (dB_value < this->tab_dB_value[2])) {

    for (int i = 0; i < NUM_LEDS / 2; i++) {
      this->leds[i] = CRGB(lum_max * tab_eyes_close[i] / 2, lum_max * tab_eyes_close[i], 0);
      this->leds[i + NUM_LEDS / 2] = CRGB(lum_max * tab_mouth_ok[i] / 2, lum_max * tab_mouth_ok[i], 0);
    }
  } else {
    for (int i = 0; i < NUM_LEDS / 2; i++) {
      this->leds[i] = CRGB(0, lum_max * tab_eyes_x[i], 0);
      this->leds[i + NUM_LEDS / 2] = CRGB(0, lum_max * tab_mouth_bad[i], 0);
    }
  }
  FastLED.show();
  usleep(10000);
}


