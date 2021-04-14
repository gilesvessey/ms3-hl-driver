#include <FastLED.h>
#define LED_PIN     1
#define NUM_LEDS    80
CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
}

// make a class that represents an RGB colour value
//
struct rgb {
  int r;
  int g;
  int b;  
};

void solid_colour(struct rgb colour) {
  leds[led] = CRGB(colour.r, colour.g, colour.b) ;

  // change the leds
  //  
  FastLED.show() ;
}

void rainbow_mode(int frequency, int change_per_tick) {
  struct rgb start = { 255, 0, 0 }  
  while (true) {
    struct rgb colour = start ;
    for (int led = 0 ; led < NUM_LEDS ; led++) {
      leds[led] = CRGB(colour.r, colour.g, colour.b) ;

      // this is how tightly packed the rainbow will be
      //
      int frequency = 8 ;
      for (int i = 0 ; i < frequency ; i++) { colour = rainbow(colour) ; }
    }
    
    // change the leds
    //
    FastLED.show() ;
      
    // progress each led in the rainbow sequence for the next update
    //
    int change_per_tick = 8 ;
    for (int i = 0 ; i < change_per_tick; i++) { start = rainbow(start) ; }
    
    // wait before we do it again
    //
    delay(25);  
  }
  struct rgb rainbow(struct rgb value) {

    // return the next rgb value in the rainbow
    //
    if (value.r == 255 && value.g == 0 && value.b != 255) {
      value.b++ ;
      return value ;
    }
    if (value.r != 0 && value.g == 0 && value.b == 255) {
      value.r-- ;
      return value ;
    }
    if (value.r == 0 && value.g != 255 && value.b == 255) {
      value.g++ ;
      return value ;
    }
    if (value.r == 0 && value.g == 255 && value.b != 0) {
      value.b-- ;
      return value ;
    }
    if (value.r != 255 && value.g == 255 && value.b == 0) {
      value.r++ ;
      return value ;
    }
    if (value.r == 255 && value.g != 0 && value.b == 0) {
      value.g-- ;
      return value ;
    }
  }
}

void loop() {
  int frequency = 8 ;
  int change_per_tick = 8 ;

  struct rgb colour_1 = { 255, 0, 255 } ;

  struct rgb colour_2 = { 255, 0, 255 } ;

  // logic for changing modes on the fly will come later
  //
  int mode = 0 ;

  switch (mode) {
    case 0:
      // mode 0 - solid colour
      solid_colour(colour_1) ;
      break;
    case 1:
      // mode 1 - solid colour moving fade (like your plane is doing)
      break;
    case 2:
      // mode 2 - single colour liquid fill
      break;
    case 3:
      // mode 3 - single colour bouncing back and forth (alternating colour maybe?)
      break;    
    case 4:
      // mode 4 - dual colour snake
      break;
    case 5:
      // mode 5 - block breaking animation - tech challenge
      break;
    case 6:
      // mode 6 - rainbow, baby
      rainbow_mode(frequency, change_per_tick) ;
      break;      
  }
}
