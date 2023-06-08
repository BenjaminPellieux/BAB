
// TODO: 
// - Rotation des gauge 
// ini WIfi dans setup
 
#include <FastLED.h>
#include "micros.h"
#include "matrice.cpp"
#include "wifi_manager.h"


#define MIC_PIN 34 //mic
#define DATA_PIN 4 //led
#define BUTTON_PIN 21
#define VAL_MID 256
#define SIZE_TAB 16
#define SIZE_HALF_TAB 8
#define DELAY 50
#define LUM_MAX 50
#define dB_MAX 130

uint16_t delay_val = DELAY;
bool state = 0;
bool tab_mem[SIZE_TAB][SIZE_TAB];
CRGB leds[NUM_LEDS];// Representation of an RGB pixel (Red, Green, Blue) 
Microphone micro;

settings_context settings_ctx;
idle_client_server wifi_idle_client_server = IDLE;

// default credentials




void set_state(){
  state = !state;
  usleep(delay_val);
}
//------------------------------------------------------------//

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(MIC_PIN, INPUT);
  Serial.begin(9600);
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
  micro.setup_mic();
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), set_state, RISING);
  init_wifi_management(&settings_ctx, &wifi_idle_client_server);
}

//------------------------------------------------------------//



static void quick_sort(uint32_t* array, uint8_t array_size){
  bool flag;
  uint16_t tmp;

  do{// sort all samples using quick short as the sample size is small
    flag = 0;
    for(uint8_t k = 0; k != array_size; k++){
      if (array[k] > array[k + 1]){
        tmp = array[k + 1];
        array[k + 1] = array[k];
        array[k] = tmp;
        flag = 1;
      }
    }
  }while(flag);

}


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
  if (sample_size <= 0){
    return EXIT_FAILURE;
  }

  for(int i = 0; i != sample_size; i++){

    sample += micro->mic_get_val();
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
static uint32_t calc_dB_median(Microphone* micro,uint32_t sample_size){
  
  if (sample_size <= 0){
   return EXIT_FAILURE;
  }

  uint32_t *sample = (uint32_t*) malloc(sizeof(uint32_t) * sample_size);

  if (!sample) // Check of thin provisioning exit if failure
    return EXIT_FAILURE; 
  
  for(int i = 0; i != sample_size; i++){// Collect all samples in a tab 
    sample[i] = micro->mic_get_val();
  }
  
  quick_sort(sample, sample_size);

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
void display_smiley(uint8_t dB_val, settings_context* settings_ctx)
{
  // smiley is 0 if db_val-70 < 0 else 1 + int(dB_val / 100)
  uint8_t smiley = ((dB_val-settings_ctx->seuil_1) < 0) ? 0 : 
                   ((dB_val - settings_ctx->seuil_2) < 0) ? 1 : 2;
  uint8_t g = dB_val * LUM_MAX / dB_MAX; // from 0 to LUM_MAX
  uint8_t r = LUM_MAX - dB_val * LUM_MAX / dB_MAX; // from LUM_MAX to 0  
  bool led;
  uint8_t j_fin, i_fin;


  // for(uint8_t i = 0; i != SIZE_HALF_TAB; i++){
  //   for(uint8_t j = 0; j != SIZE_HALF_TAB; j++){

  //     led = tab_eyes[smiley][i + j * SIZE_HALF_TAB];
  //     //led = tab_mouth[smiley][i + j * SIZE_TAB];

  //     if(!i % 2){
  //       i_fin = SIZE_TAB-1-i;
  //     }
  //     else{
  //       i_fin = i + j * SIZE_HALF_TAB;
  //     }

  //     leds[i_fin] = CRGB(r * led, g * led, 0); 

  //   }      
  // }



  for(uint8_t i = 0; i != MID_NUM_LED; i++ ){
    led = tab_eyes[smiley][i];
    //leds[i + 255 - (i % SIZE_TAB) * (SIZE_TAB)] = CRGB(g * led, r * led, 0);
    leds[i] = CRGB(r * led, g * led, 0);
    led = tab_mouth[smiley][i];
    leds[i + MID_NUM_LED] = CRGB(r * led, g * led, 0);

  }
  FastLED.show();
}

//------------------------------------------------------------//


/*
Fill tab based on current volume
Matrice size:  16 * 16
dB_MAX / 16 = dB per Pixel  
This function decomposes all the rows of the table and fills 
the last according to the volume found
@ Author: Pierre BABIAN & Benjamin PELLIEUX
@ Date:  31/05/23
@ State:  Done 
*/
void fill_tab(uint32_t dB_val, settings_context* settings_ctx)
{
  bool buf_case;
  bool buf_line[SIZE_TAB] = {0};
  
  for (uint8_t i = 0; i !=  SIZE_TAB; i++) {
    for (uint8_t j = 0; j != SIZE_TAB; j++) {   
      if (i) {
        buf_case = tab_mem[i][j];
        tab_mem[i][j] = buf_line[j];
        buf_line[j] = buf_case;
      } else {
        buf_line[j] = tab_mem[i][j];
        if (j < (dB_val * settings_ctx->sensitivity * SIZE_TAB) / dB_MAX) {
          tab_mem[i][j] = 1;
        } else {
          tab_mem[i][j] = 0;
        }
      }
    }
  }
}  


// DEPRECATED 
// fonction d'affchage des jauges Deprecated 
// void display_gauge()
// {
//   uint8_t r = 0, g = 0;

//   for (uint8_t i = 0; i != SIZE_TAB; i++) {
//     for (uint8_t j = 0; j != SIZE_TAB; j++) {
//       if (tab_mem[i][j]) {
//         r = (LUM_MAX * j) / SIZE_TAB;
//         g = (LUM_MAX * (SIZE_TAB - j)) / SIZE_TAB;
//       } else {
//         r = 0; g = 0;
//       }
//       leds[(i * SIZE_TAB) + j] = CRGB((r * (i % 2)) + (g * !(i % 2)),
//                                       (r * !(i % 2)) + (g * (i % 2)),
//                                       0);
//     }
//   }
//   FastLED.show(); 
// }

// fonction d'affchage des jauges 
void display_gauge()
{
  uint8_t r = 0, g = 0, used_j = 0;

  for (uint8_t i = 0; i != SIZE_TAB; i++) {
    for (uint8_t j = 0; j != SIZE_TAB; j++) {
      used_j = (i % 2) ? SIZE_TAB - j - 1 : j;
      
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
  wifi_management(&settings_ctx, &wifi_idle_client_server);
  if (state){
    display_smiley(calc_dB_average(&micro,35), &settings_ctx);
  }else{
    fill_tab(calc_dB_average(&micro,5),&settings_ctx);
    display_gauge();
  }
  usleep(delay_val);
}
