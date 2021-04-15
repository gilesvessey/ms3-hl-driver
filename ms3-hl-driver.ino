#include <FastLED.h>
#define LED_PIN     1
#define NUM_LEDS    43
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

void solid_colour_mode(struct rgb colour) {
  for (int led = 0; led < NUM_LEDS; led++) {
    leds[led] = CRGB(colour.r, colour.g, colour.b);
  }

  // change the leds
  //  
  FastLED.show();
}

void pulse_mode(struct rgb colour, int frequency, double speed) {
  bool brightening = false;
  struct rgb curr = colour;
  while(true) {
    for (int led = 0; led < NUM_LEDS; led++) {

    }
  }
}

void liquid_fill_mode(struct rgb colour, double speed) {
  // TODO adjustable drip size, animation takes a long time and will only take longer with 144/m
  //
  int pool = 0;
  int drip = NUM_LEDS;

  while(true) {
    for (int led = 0; led < NUM_LEDS; led++) {
      if (led < pool) {

        // render the pool at one end
        //
        leds[led] = CRGB(colour.r, colour.g, colour.b);
      } else if (led == drip) {

        // fill in the drip
        //
        leds[led] = CRGB(colour.r, colour.g, colour.b);
      } else {

        // otherwise a blank pixel
        //
        leds[led] = CRGB(0, 0, 0);
      }
    }

    // change the leds
    //
    FastLED.show();

    if (pool == NUM_LEDS) {

      // the pool is full, reset
      //
      pool = 0;
      drip = NUM_LEDS;
    }

    drip--;
    if (drip == pool) {

      // the drip has landed, I repeat, the drip has landed
      //
      pool++;
      drip = NUM_LEDS;
    }

    // wait before we do it again
    // range is 5 - 15
    //
    delay(5 + (10 * speed));     
  }
}

void bounce_mode(struct rgb colour, bool isRainbow, double speed, int frequency) {
  int ball = 0;
  bool rising;
  while(true) {
    if (isRainbow) { 
      for (int i = 0; i < frequency; i++) { colour = rainbow(colour); } 
    }
    for (int led = 0; led < NUM_LEDS; led++) {
      if (led == ball) {
        leds[led] = CRGB(colour.r, colour.g, colour.b);
      } else {
        leds[led] = CRGB(0, 0, 0);
      }
    }
      
      // change the leds
      //
      FastLED.show();

      // do bouncy ball things
      //
      if (ball == 0) { rising = true; }
      if (ball == NUM_LEDS) { rising = false; }

      if (rising) { ball++; }
      else { ball--; }

    // wait before we do it again
    // range is 20 - 200ms
    //
    delay(20 + (180 * speed));  
  }
}

struct rgb pulse(struct rgb colour, int frequency, struct rgb curr, bool brightening) {
  // frequency of 8 is 8/255ths change each tick
  //
  double multiplier = 1 + (frequency / 255);
  if (brightening) {
    curr.r = curr.r * multiplier; 
    if (curr.r > colour.r) { curr.r = colour.r; }
    curr.g = curr.g * multiplier; 
    if (curr.g > colour.g) { curr.g = colour.g; }    
    curr.b = curr.b * multiplier; 
    if (curr.b > colour.b) { curr.b = colour.b; }      
  } else {
    curr.r = curr.r / multiplier; 
    if (curr.r < colour.r) { curr.r = colour.r; }
    curr.g = curr.g / multiplier; 
    if (curr.g < colour.g) { curr.g = colour.g; }    
    curr.b = curr.b / multiplier; 
    if (curr.b < colour.b) { curr.b = colour.b; }  
  }
  return curr;
}

struct rgb rainbow(struct rgb value) {

  // return the next rgb value in the rainbow
  //
  if (value.r == 255 && value.g == 0 && value.b != 255) {
    value.b++;
    return value;
  } else if (value.r != 0 && value.g == 0 && value.b == 255) {
    value.r--;
    return value;
  } else if (value.r == 0 && value.g != 255 && value.b == 255) {
    value.g++;
    return value;
  } else if (value.r == 0 && value.g == 255 && value.b != 0) {
    value.b--;
    return value;
  } else if (value.r != 255 && value.g == 255 && value.b == 0) {
    value.r++;
    return value;
  } else if (value.r == 255 && value.g != 0 && value.b == 0) {
    value.g--;
    return value;
  }
} 

void rainbow_mode(double speed, int frequency) { 
  struct rgb start = { 255, 0, 0 };
  while (true) {
    struct rgb colour = start;
    for (int led = 0; led < NUM_LEDS; led++) {
      leds[led] = CRGB(colour.r, colour.g, colour.b);

      // this is how tightly packed the rainbow will be
      //
      for (int i = 0; i < frequency; i++) { colour = rainbow(colour); }
    }
    
    // change the leds
    //
    FastLED.show();
      
    // progress each led in the rainbow sequence for the next update
    // range is 4 to 12
    //
    int change_per_tick = 12 - (8 * speed);
    for (int i = 0; i < change_per_tick; i++) { start = rainbow(start); }
    
    // wait before we do it again
    //
    delay(25);  
  }
}

void loop() {
  struct rgb colour_1 = { 255, 255, 255 };

  struct rgb colour_2 = { 255, 0, 255 };

  // logic for changing modes on the fly will come later
  //
  int mode = 0;

  switch (mode) {
    case 0:
      // mode 0 - solid colour
      solid_colour_mode(colour_1);
      break;
    case 1:
      // mode 1 - solid colour moving fade (like your plane is doing)
      break;
    case 2:
      // mode 2 - single colour liquid fill
      liquid_fill_mode(colour_1, 0.5);
      break;
    case 3:
      // mode 3 - bouncing back and forth
      bounce_mode(colour_1, true, 0.5, 8);
      break;    
    case 4:
      // mode 4 - dual colour snake
      break;
    case 5:
      // mode 5 - block breaking animation - tech challenge
      break;
    case 6:
      // mode 6 - single colour strobe
      break;      
    case 7:
      // mode 6 - rainbow, baby
      rainbow_mode(0.5, 8);
      break;      
  }
}
