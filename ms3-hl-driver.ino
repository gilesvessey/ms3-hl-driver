#include <AltSoftSerial.h>
AltSoftSerial BTserial; 

char c=' ';
boolean NL = true;

#include <FastLED.h>
#define LED_PIN     4
#define NUM_LEDS    64
CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(9600);  
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  BTserial.begin(9600);  
  Serial.println("BTserial started at 9600");  
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

struct ls { int pool; int drip; };
struct ls liquid_fill_mode(struct rgb colour, double speed, struct ls state) {
  // TODO adjustable drip size, animation takes a long time and will only take longer with 144/m
  //

    for (int led = 0; led < NUM_LEDS; led++) {
      if (led < state.pool) {

        // render the pool at one end
        //
        leds[led] = CRGB(colour.r, colour.g, colour.b);
      } else if (led == state.drip) {

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

    if (state.pool == NUM_LEDS) {

      // the pool is full, reset
      //
      state.pool = 0;
      state.drip = NUM_LEDS;
    }

    state.drip--;
    if (state.drip == state.pool) {

      // the drip has landed, I repeat, the drip has landed
      //
      state.pool++;
      state.drip = NUM_LEDS;
    }

  return state;
}

struct bs { int ball; bool rising; struct rgb curr; };
struct bs bounce_mode(bool isRainbow, int frequency, struct bs state) {
  if (isRainbow) 
    for (int i = 0; i < frequency; i++) state.curr = rainbow(state.curr);

  for (int led = 0; led < NUM_LEDS; led++) {
    if (led == state.ball) 
      leds[led] = CRGB(state.curr.r, state.curr.g, state.curr.b);
    else 
      leds[led] = CRGB(0, 0, 0);
  }
  
  // change the leds
  //
  FastLED.show();

  // do bouncy ball things
  //
  if (state.ball == 0) 
    state.rising = true; 
  else if (state.ball == NUM_LEDS) 
    state.rising = false; 

  if (state.rising) 
    state.ball++; 
  else 
    state.ball--; 

  return state;
}

struct rgb pulse(struct rgb colour, int frequency, struct rgb curr, bool brightening) {
  // frequency of 8 is 8/255ths change each tick
  //
  if (brightening) {
    curr.r = curr.r + frequency; 
    if (curr.r > colour.r) { curr.r = colour.r; }
    curr.g = curr.g + frequency; 
    if (curr.g > colour.g) { curr.g = colour.g; }    
    curr.b = curr.b + frequency; 
    if (curr.b > colour.b) { curr.b = colour.b; }      
  } else {
    curr.r = curr.r - frequency; 
    if (curr.r < 0) { curr.r = 0; }
    curr.g = curr.g - frequency; 
    if (curr.g < 0) { curr.g = 0; }    
    curr.b = curr.b - frequency; 
    if (curr.b < 0) { curr.b = 0; }  
  }
  return curr;
}
struct ps { struct rgb curr; bool brightening; };
struct ps pulse_mode(struct rgb colour, int frequency, struct ps state) {
  struct rgb curr = state.curr;
  bool brightening = state.brightening;
  for (int led = 0; led < NUM_LEDS; led++) {
    if (curr.r == 0 && curr.g == 0 && curr.b == 0) {
      brightening = true;
      curr = pulse(colour, frequency, curr, brightening);           
    } else if (curr.r == colour.r && curr.g == colour.g && curr.b == colour.b) {
      brightening = false;
    }

    leds[led] = CRGB(curr.r, curr.g, curr.b);
    curr = pulse(colour, frequency, curr, brightening); 
  }

  // progress in the sequence one more time so we shift on the next tick
  state.curr = pulse(colour, frequency, state.curr, state.brightening); 
    if (state.curr.r == colour.r && state.curr.g == colour.g && state.curr.b == colour.b) 
      state.brightening = false;
    else if (state.curr.r == 0 && state.curr.g == 0 && state.curr.b == 0) 
      state.brightening = true;  
  // change the leds
  //
  FastLED.show();

  return state;
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

struct rgb rainbow_mode(double speed, int frequency, struct rgb state) { 
    struct rgb colour = state;
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
    for (int i = 0; i < change_per_tick; i++) { state = rainbow(state); }
    
    return state;
}
int mode = 3; 
struct rgb colour_1 = { 255, 0, 255 };
struct ls liquid_state = { 0, NUM_LEDS };
struct rgb rainbow_state = { 255, 0, 0 };
struct bs bounce_state = { 0, true, { 255, 0, 0 } };
struct ps pulse_state = { { 255, 0, 255 }, false };
void loop() {
  // Read from the Bluetooth module and send to the Arduino Serial Monitor
  if (BTserial.available())
  {
      c = BTserial.read();
      Serial.write(c);
      if (isdigit(c)) mode = c - '0';
  }
     // Read from the Serial Monitor and send to the Bluetooth module
    if (Serial.available())
    {
        c = Serial.read();
 
        Serial.write(c);
 
        // do not send line end characters to the HM-10
        if (c != 10 && c != 13) 
        {  
             BTserial.write(c);
        }
 
        // Echo the user input to the main window. 
        // If there is a new line print the ">" character.
        if (NL) Serial.print("\r\n>");  NL = false; 
  
        Serial.write(c);

        if (c==10) NL = true;
    }

  switch (mode) {
    case 0:
      // mode 0 - solid colour
      solid_colour_mode(colour_1);
      break;
    case 1:
      // mode 1 - solid colour moving fade
      pulse_state = pulse_mode(colour_1, 6, pulse_state);
      break;
    case 2:
      // mode 2 - single colour liquid fill
      liquid_state = liquid_fill_mode(colour_1, 0.5, liquid_state);
      break;
    case 3:
      // mode 3 - bouncing back and forth
      bounce_state = bounce_mode(true, 8, bounce_state);
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
      // mode 7 - rainbow, baby
      rainbow_state = rainbow_mode(0.5, 8, rainbow_state);
      break;      
  }
  // wait before we do it again
  //
  delay(50);
}
