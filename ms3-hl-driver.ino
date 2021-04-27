#include <Regexp.h>
#include <string.h>

#include <AltSoftSerial.h>
AltSoftSerial BTserial; 

// #include <SoftwareSerial.h>
// SoftwareSerial BTserial(8, 9);

boolean NL = true;
char data[20];

#include <FastLED.h>
#define LED_PIN     4
#define NUM_LEDS    64
CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(9600);  
  BTserial.begin(9600);    
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  Serial.println("ms3-hl-driver 1.0.0");  
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
    if (curr.r > colour.r) curr.r = colour.r;
    curr.g = curr.g + frequency; 
    if (curr.g > colour.g) curr.g = colour.g;    
    curr.b = curr.b + frequency; 
    if (curr.b > colour.b) curr.b = colour.b;      
  } else {
    curr.r = curr.r - frequency; 
    if (curr.r < 0) curr.r = 0;
    curr.g = curr.g - frequency; 
    if (curr.g < 0) curr.g = 0;    
    curr.b = curr.b - frequency; 
    if (curr.b < 0) curr.b = 0;  
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
  //
  state.curr = pulse(colour, frequency, state.curr, state.brightening); 
  if (state.curr.r == colour.r && state.curr.g == colour.g && state.curr.b == colour.b) 
    state.brightening = false;
  else if (state.curr.r == 0 && state.curr.g == 0 && state.curr.b == 0) 
    state.brightening = true;  

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
  } else {

    // a value was supplied that is not a part of the rainbow sequence, set it to red to get it started
    //
    struct rgb start = { 255, 0, 0 };
    return start;
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
  FastLED.show();
    
  for (int i = 0; i < 8; i++) { state = rainbow(state); }
  
  return state;
}
int mode = 3; 
struct rgb colour_1 = { 255, 0, 255 };
struct ls liquid_state = { 0, NUM_LEDS };
struct rgb rainbow_state = { 255, 0, 0 };
struct bs bounce_state = { 0, true, { 255, 0, 0 } };
struct ps pulse_state = { { 255, 0, 255 }, false };

void loop() {
  // we need to be able to accept a command that sets colour
  // and mode
  // and frequency (denseness of flows)
  // and speed (refresh rate)

  // clear the input value
  memset(data, 0, 20);

  // gather the input string
  for (int i = 0; BTserial.available(); i++) {
    data[i] = BTserial.read();
    // wait before reading another byte, processor speed is faster than serial sending speed
    // so this ensures that we read 100% of an incoming command
    //
    delay(1);
  }

  if (strlen(data) > 0) {
    Serial.println("Received data via bluetooth:");
    Serial.println(data);
    if (data[0] == 'm' || data[0] == 'M') {

      // set the mode, 'm 0'
      //
      mode = data[2] - '0';
    }
    if (data[0] == 'c' || data[0] == 'C') {

      // set the colour, 'c 255 0 255'
      //

      // ISSUES: (works 60% of the time)
      // sometimes the incoming data is split into two occurences, neither of which are interpretable
      // sometimes even though the correct value is read over bluetooth, the parsing is demonstrating inconsistent results somehow
      //
      struct rgb colour = { 256, 256, 256 };
      // clear the buffer for the next colour change
      //    
      char buff[4];
      memset(buff, 0, 4);        
      // convoluted way of reading three integers delimited by spaces
      //
      for (int i = 2; i < strlen(data); i++) {
        if (data[i] == ' ') {
          buff[strlen(buff)] = '\0';
          if (colour.r == 256) colour.r = atoi(buff);
          else if (colour.g == 256) colour.g = atoi(buff);
          else if (colour.b == 256) colour.b = atoi(buff);          
          // clear the buffer for the next number
          //
          memset(buff, 0, 4);  
          // skip over the space
          //
          i++;                          
        }
        buff[strlen(buff)] = data[i]; 
      }
      if (colour.r != 256 && colour.g != 256 && colour.b != 256) {  
        // all of the colour values have been set
        //
        Serial.println("setting colour to: ");
        Serial.println(colour.r);
        Serial.println(colour.g);
        Serial.println(colour.b);
        colour_1 = colour;
      }

    }    
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
