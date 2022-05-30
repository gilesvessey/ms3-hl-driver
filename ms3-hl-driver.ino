#include <string.h>
#include <AltSoftSerial.h>
#include <FastLED.h>
#define DRIVER_PIN 5
#define PASS_PIN 4
#define STRIP_LEDS 59
#define HALO_LEDS 32
#define ALL_LEDS 91

AltSoftSerial BTserial;
CRGB leds[ALL_LEDS];
CRGB pass_leds[ALL_LEDS];
CRGB driver_leds[ALL_LEDS];
boolean NL = true;
char data[20];

void setup() {
  Serial.begin(9600);
  BTserial.begin(9600);
  FastLED.addLeds<WS2812, DRIVER_PIN, GRB>(leds, ALL_LEDS);
  FastLED.addLeds<WS2812, PASS_PIN, GRB>(leds, ALL_LEDS);

  FastLED.addLeds<WS2812, DRIVER_PIN, GRB>(driver_leds, ALL_LEDS);
  FastLED.addLeds<WS2812, PASS_PIN, GRB>(pass_leds, ALL_LEDS);

  Serial.println("ms3-hl-driver 1.0.0");
}

// make a class that represents an RGB colour value
//
struct rgb {
  int r;
  int g;
  int b;
};

void solid_colour_mode(struct rgb strip, struct rgb halo) {

  // set the led strip colour
  //
  for (int led = HALO_LEDS; led < ALL_LEDS; led++) {
    leds[led] = CRGB(strip.r, strip.g, strip.b);
  }

  // set the halo colour
  //
  for (int led = 0; led < HALO_LEDS; led++) {
    leds[led] = CRGB(halo.r, halo.g, halo.b);
  }

  FastLED.show();
}

struct ls { int pool; int drip; };
struct ls liquid_fill_mode(struct rgb strip, struct rgb halo, double speed, struct ls state) {
  // TODO adjustable drip size, animation takes a long time and will only take longer with 144/m
  //

  for (int led = HALO_LEDS; led < ALL_LEDS; led++) {
    if (led < state.pool) {

      // render the pool at one end
      //
      leds[led] = CRGB(strip.r, strip.g, strip.b);
    } else if (led == state.drip) {

      // fill in the drip
      //
      leds[led] = CRGB(strip.r, strip.g, strip.b);
    } else {

      // otherwise a blank pixel
      //
      leds[led] = CRGB(0, 0, 0);
    }
  }
  FastLED.show();

  if (state.pool == ALL_LEDS) {

    // the pool is full, reset
    //
    state.pool = HALO_LEDS;
    state.drip = ALL_LEDS;
  }

  state.drip--;
  if (state.drip == state.pool) {

    // the drip has landed, I repeat, the drip has landed
    //
    state.pool++;
    state.drip = ALL_LEDS;
  }

  // set the halo colour
  //
  for (int led = 0; led < HALO_LEDS; led++) {
    leds[led] = CRGB(halo.r, halo.g, halo.b);
  }

  return state;
}

struct bs { int ball; bool rising; struct rgb curr; };
struct bs bounce_mode(bool isRainbow, int frequency, struct rgb halo, struct bs state) {
  if (isRainbow)
    for (int i = 0; i < frequency; i++) state.curr = rainbow(state.curr);

  // set the strip
  //
  for (int led = HALO_LEDS; led < ALL_LEDS; led++) {
    if (led == state.ball)
      leds[led] = CRGB(state.curr.r, state.curr.g, state.curr.b);
    else
      leds[led] = CRGB(0, 0, 0);
  }

  // set the halo
  //
  for (int led = 0; led < HALO_LEDS; led++) {
    leds[led] = CRGB(halo.r, halo.g, halo.b);
  }

  FastLED.show();

  // do bouncy ball things
  //
  if (state.ball == HALO_LEDS)
    state.rising = true;
  else if (state.ball == ALL_LEDS)
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

struct ps { bool brightening; struct rgb curr; };
struct ps pulse_mode(struct rgb colour, int frequency, struct ps state) {
  struct rgb curr = state.curr;
  bool brightening = state.brightening;
  for (int led = 0; led < ALL_LEDS; led++) {
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

  for (int led = 0; led < ALL_LEDS; led++) {
    leds[led] = CRGB(colour.r, colour.g, colour.b);

    // this is how tightly packed the rainbow will be
    //
    for (int i = 0; i < frequency; i++) { colour = rainbow(colour); }
  }
  FastLED.show();

  for (int i = 0; i < 8; i++) { state = rainbow(state); }

  return state;
}

int loop_de_loop(boolean isPassSide, int pos) {
  if (isPassSide && pos > 0 && pos <= 32) { // reverse the halo for the pass side
    pos--;
  } else {
    pos++;
  }

  if (pos == 55) {
    if (isPassSide) {
      pos = 32;
    } else {
      pos = 0;
    }
  }
  if (pos == 0 && isPassSide) {
    pos = 56;
  }
  if (pos == 32 && !isPassSide) {
    pos = 56;
  }
  if (pos == 91) {
    pos = 33;
  }

  return pos;
}

struct ss { int driverBall; int passBall; struct rgb curr; boolean isFinale; boolean done; };
struct ss start_mode(struct ss state) {
  int offset = 5;
  for (int led = 0; led < ALL_LEDS; led++) {

    // munge the led position for the passenger's side halo
    //
    int offset = 5;
    int pass_offset_led = led;
    int driver_offset_led = led;
    if (led < 32) {
      for (int i = 0; i < offset; i++) {
        if (pass_offset_led == 31) {
          pass_offset_led = 0;
        } else {
          pass_offset_led++;
        }

        if (driver_offset_led == 0) {
          driver_offset_led = 31;
        } else {
          driver_offset_led--;
        }
      }
    }

    if (led == state.passBall || led == state.passBall - 1 || led == state.passBall + 1) { // if this is the location of the ball, light it up
      pass_leds[pass_offset_led] = CRGB(state.curr.r, state.curr.g, state.curr.b);
    } else {
      if (!state.isFinale) {
        pass_leds[pass_offset_led] = CRGB(0, 0, 0);
      }
    }

    if (led == state.driverBall || led == state.driverBall - 1 || led == state.driverBall + 1) { // if this is the location of the ball, light it up
      driver_leds[driver_offset_led] = CRGB(state.curr.r, state.curr.g, state.curr.b);
    } else {
      if (!state.isFinale) {
        driver_leds[driver_offset_led] = CRGB(0, 0, 0);
      }
    }
  }

  FastLED.show();

  if (state.isFinale) {
    state.driverBall--;
  } else {
    state.driverBall = loop_de_loop(false, state.driverBall);
  }

  if (state.isFinale) {
    state.passBall--;
  } else {
    state.passBall = loop_de_loop(true, state.passBall);
  }
  // we ready for the finale?
  //
  if (state.driverBall == 90 || state.passBall == 90) {
    state.isFinale = true;
  }
  // ready to rip boys
  //
  if ((state.driverBall == 0 || state.passBall == 0) && state.isFinale) {
    state.done = true;
  }

  return state;
}

struct rgb read_rgb(char data[], int first_digit) {
  // set the colour, 'c 255 0 255'
  //
  struct rgb colour = { 256, 256, 256 };
  // clear the buffer for the next colour change
  //
  char buff[4];
  memset(buff, 0, 4);
  // convoluted way of reading three integers delimited by spaces
  //
  for (int i = first_digit; i < strlen(data); i++) {
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
    // all values set
    //
    return colour;
  } else {
    // something went wrong
    //
    struct rgb fallback = { 255, 255, 255};
    return fallback;
  }
}

// DEFAULTS
//
int mode = 8;
struct rgb colour_strip = { 255, 69, 0 }; // daily driving orange
struct rgb colour_halo = { 255, 50, 0 }; // daily driving orange


// struct rgb colour_strip = { 0, 255, 0 };

// state for each mode that requires it
//
struct ls liquid_state = { HALO_LEDS, ALL_LEDS };
struct rgb rainbow_state = { 255, 0, 0 };
struct bs bounce_state = { 0, true, { 255, 0, 0 } };
struct ps pulse_state = { false, { 255, 0, 0 } };
struct ss start_state = { 0, 32, { 255, 0, 255 }, false, false };

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
      switch(data[1]) {
        case 's': // set the strip
          colour_strip = read_rgb(data, 3);
          break;
        case 'h': // set the halo
          colour_halo = read_rgb(data, 3);
          break;
        case ' ': // none specified, set both
          colour_strip = read_rgb(data, 2);
          colour_halo = colour_strip;
          break;
      }
    }
  }

  switch (mode) {
    case 0:
      // mode 0 - solid colour
      solid_colour_mode(colour_strip, colour_halo);
      break;
    case 1:
      // mode 1 - solid colour moving fade
      pulse_state = pulse_mode(colour_strip, 6, pulse_state);
      break;
    case 2:
      // mode 2 - single colour liquid fill
      liquid_state = liquid_fill_mode(colour_strip, colour_halo, 0.5, liquid_state);
      break;
    case 3:
      // mode 3 - bouncing back and forth
      bounce_state = bounce_mode(true, 8, colour_halo, bounce_state);
      break;
    case 4:
      // mode 4 - dual colour snake
      break;
    case 5:
      // mode 5 -
      break;
    case 6:
      // mode 6 - single colour strobe
      break;
    case 7:
      // mode 7 - rainbow, baby
      rainbow_state = rainbow_mode(0.5, 8, rainbow_state);
      break;
    case 8:
      // mode 8 - startup sequence
      // if (start_state.done) {
      //   mode = 0
      // }
      start_state = start_mode(start_state);
      break;
  }
  // wait before we do it again
  //
  delay(10);
}
