#include <string.h>
#include <AltSoftSerial.h>
#include <FastLED.h>
#define DRIVER_PIN 5
#define PASS_PIN 4
#define GLOW_PIN 6

#define STRIP_LEDS 59
#define HALO_LEDS 32
#define ALL_LEDS 91
#define GLOW_LEDS 205

AltSoftSerial BTserial;
CRGB pass_leds[ALL_LEDS];
CRGB driver_leds[ALL_LEDS];
CRGB glow_leds[GLOW_LEDS];

boolean NL = true;
char data[20];

void setup() {
  Serial.begin(9600);
  BTserial.begin(9600);

  FastLED.addLeds<WS2812, DRIVER_PIN, GRB>(driver_leds, ALL_LEDS);
  FastLED.addLeds<WS2812, PASS_PIN, GRB>(pass_leds, ALL_LEDS);
  FastLED.addLeds<WS2812, GLOW_PIN, GRB>(glow_leds, GLOW_LEDS);

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

  set the led strip colour

  for (int led = HALO_LEDS; led < ALL_LEDS; led++) {
    pass_leds[led] = CRGB(strip.r, strip.g, strip.b);
    driver_leds[led] = CRGB(strip.r, strip.g, strip.b);
  }

  // set the halo colour
  //
  for (int led = 0; led < HALO_LEDS; led++) {
    pass_leds[led] = CRGB(halo.r, halo.g, halo.b);
    driver_leds[led] = CRGB(strip.r, strip.g, strip.b);
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
      pass_leds[led] = CRGB(strip.r, strip.g, strip.b);
      driver_leds[led] = CRGB(strip.r, strip.g, strip.b);

    } else if (led == state.drip) {

      // fill in the drip
      //
      pass_leds[led] = CRGB(strip.r, strip.g, strip.b);
      driver_leds[led] = CRGB(strip.r, strip.g, strip.b);

    } else {

      // otherwise a blank pixel
      //
      pass_leds[led] = CRGB(0, 0, 0);
      driver_leds[led] = CRGB(0, 0, 0);
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
    pass_leds[led] = CRGB(halo.r, halo.g, halo.b);
    driver_leds[led] = CRGB(halo.r, halo.g, halo.b);
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
    if (led == state.ball) {
      driver_leds[led] = CRGB(state.curr.r, state.curr.g, state.curr.b);
      pass_leds[led] = CRGB(state.curr.r, state.curr.g, state.curr.b);
    }
    else {
      driver_leds[led] = CRGB(0, 0, 0);
      pass_leds[led] = CRGB(0, 0, 0);
    }
  }

  // set the halo
  //
  for (int led = 0; led < HALO_LEDS; led++) {
    driver_leds[led] = CRGB(halo.r, halo.g, halo.b);
    pass_leds[led] = CRGB(halo.r, halo.g, halo.b);
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
    pass_leds[led] = CRGB(colour.r, colour.g, colour.b);
    driver_leds[led] = CRGB(colour.r, colour.g, colour.b);


    // this is how tightly packed the rainbow will be
    //
    for (int i = 0; i < frequency; i++) { colour = rainbow(colour); }
  }

    for (int led = 0; led < GLOW_LEDS; led++) {
      glow_leds[led] = CRGB(colour.r, colour.g, colour.b);


    // this is how tightly packed the rainbow will be
    //
    for (int i = 0; i < frequency; i++) { colour = rainbow(colour); }
  }


  FastLED.show();

  for (int i = 0; i < 8; i++) { state = rainbow(state); }

  return state;
}

int loop_de_loop(boolean isPassSide, int pos) {
  if (isPassSide && pos > 0 && pos <= 31)  // reverse the halo for the pass side
    pos--;
  else
    pos++;

  // entering into the loop
  //
  if (pos == 55) {
    if (isPassSide)
      pos = 31;
    else
      pos = 0;
  }

  // exiting the loop
  //
  if ((pos == 0 && isPassSide) || (pos == 31 && !isPassSide))
    pos = 56;

  return pos;
}

int loop_de_loop_reverse(boolean isPassSide, int pos) {
  if (isPassSide && pos > 0 && pos <= 31)  // reverse the halo for the pass side
    pos++;
  else
    pos--;

  // entering into the loop
  //
  if (pos == 56) {
    if (isPassSide)
      pos = 0;
     else
      pos = 31;
  }

  // exiting the loop
  //
  if ((pos == 31 && isPassSide) || (pos == 0 && !isPassSide))
    pos = 55;

  return pos;
}

struct ss { int driverBall; int passBall; struct rgb curr; boolean isFinale; boolean done; };
struct ss start_mode(struct ss state) {
  int offset = 5;
  for (int led = 0; led < ALL_LEDS; led++) {
    // munge the led position for the halos
    //
    int offset = 5;
    int pass_offset_led = led;
    int driver_offset_led = led;
    if (led <= 31) {
      for (int i = 0; i < offset; i++) {
        if (pass_offset_led == 31)
          pass_offset_led = 0;
        else
          pass_offset_led++;

        if (driver_offset_led == 0)
          driver_offset_led = 31;
        else
          driver_offset_led--;
      }
    }

    if (led == state.passBall || led == state.passBall - 1 || led == state.passBall + 1)  // if this is the location of the ball, light it up
      pass_leds[pass_offset_led] = CRGB(state.curr.r, state.curr.g, state.curr.b);
    else if (!state.isFinale)
      pass_leds[pass_offset_led] = CRGB(0, 0, 0);

    if (led == state.driverBall || led == state.driverBall - 1 || led == state.driverBall + 1)  // if this is the location of the ball, light it up
      driver_leds[driver_offset_led] = CRGB(state.curr.r, state.curr.g, state.curr.b);
    else if (!state.isFinale)
      driver_leds[driver_offset_led] = CRGB(0, 0, 0);
  }

  if (state.isFinale) {
    state.driverBall--;
    state.passBall--;
  } else {
    state.driverBall = loop_de_loop(false, state.driverBall);
    state.passBall = loop_de_loop(true, state.passBall);
  }

  // finished the loopdeloop, begin filling everything in
  //
  if (state.driverBall == 90 || state.passBall == 90)
    state.isFinale = true;

  // start sequence complete
  //
  if (state.isFinale && (state.driverBall == 0 || state.passBall == 0))
    state.done = true;

  FastLED.show();
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
int glode = 0;
// struct rgb colour_strip = { 255, 69, 0 }; // daily driving orange
// struct rgb colour_halo = { 255, 69, 0 }; // daily driving orange

struct rgb colour_strip = { 50, 50, 255 }; // cool blue
struct rgb colour_halo = { 50, 50, 255 }; // cool blue
struct rgb colour_glow = { 50, 50, 255 }; // cool blue

// state for each mode that requires it
//
struct ls liquid_state = { HALO_LEDS, ALL_LEDS };
struct rgb rainbow_state = { 255, 0, 0 };
struct bs bounce_state = { 0, true, { 255, 0, 0 } };
struct ss start_state = { 32, 32, { 50, 50, 255 }, false, false };
struct sps { int driverBall; int passBall; }
struct sps sparkle_state = { 50, 50 }
int circle_state = 0;

void loop() {
  // clear the input value
  memset(data, 0, 20);

  // gather the input string
  for (int i = 0; BTserial.available(); i++) {
    data[i] = BTserial.read();
    // wait before reading another byte, processor speed is faster than serial sending speed
    // so this ensures that we read 100% of an incoming command
    //
    delay(1); // experiment - setting this to 2ms, commands are hit and miss
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
        case 'g': // set the glow
          colour_glow = read_rgb(data, 3);
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
      // mode 1 - unused
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
      // mode 4 - unused
      break;
    case 5:
      // mode 5 - unused
      break;
    case 6:
      // mode 6 - unused
      break;
    case 7:
      // mode 7 - rainbow, baby
      rainbow_state = rainbow_mode(0.5, 8, rainbow_state);
      break;
    case 8:
      // mode 8 - startup sequence
      if (start_state.done == true) // allow it to be run more than once
        // start_state = { 0, 32, { 255, 50, 0 }, false, false };
        start_state = { 33, 33, { 255, 50, 0 }, false, false };


      start_state = start_mode(start_state);

      if (start_state.done == true) // sequence is over, daily driving time
        mode = 0;

      break;
  }

  // do something with the underglow
  //
  switch(glode) {
    case 0: // just glowin'
      for (int led = 0; led < GLOW_LEDS; led++) {
        glow_leds[led] = CRGB(colour_glow.r, colour_glow.g, colour_glow.b);
      }
      FastLED.show();
      break;
    case 1: // circling the car
      // turn everything off
      for (int led = 0; led < GLOW_LEDS; led++) {
          glow_leds[led] = CRGB(0, 0, 0);
      }
      // only turn on circle_state with a margin of 2 leds on each side
      for (int led = 0; led < GLOW_LEDS; led++) {
        if (led == circle_state) {
            glow_leds[led - 2] = CRGB(colour_glow.r, colour_glow.g, colour_glow.b);
            glow_leds[led - 1] = CRGB(colour_glow.r, colour_glow.g, colour_glow.b);
            glow_leds[led] = CRGB(colour_glow.r, colour_glow.g, colour_glow.b);
            glow_leds[led + 1] = CRGB(colour_glow.r, colour_glow.g, colour_glow.b);
            glow_leds[led + 2] = CRGB(colour_glow.r, colour_glow.g, colour_glow.b);
        }
      }

      if (circle_state >= GLOW_LEDS) circle_state = 0;
      circle_state++;

      FastLED.show();
      break;
    case 2: // sparkles originating from front
      for (int led = 0; led < GLOW_LEDS; led++) {
        if (led == sparkle_state.driverBall || led == sparkle_state.passBall) {
          glow_leds[led] = CRGB(colour_glow.r, colour_glow.g, colour_glow.b);
        } else {
          glow_leds[led] = CRGB(0, 0, 0);
        }
      }

      FRONT_CENTER_OFFSET = 50
      REAR_CENTER_OFFSET = 40

      if (sparkle_state.driverBall == 0) // patch for making it jump over the drivers wheel
        sparkle_state.driverBall = GLOW_LEDS

      if (sparkle_state.driverBall == (GLOW_LEDS - REAR_CENTER_OFFSET))
        sparkle_state.driverBall = FRONT_CENTER_OFFSET
      if (sparkle_state.passBall == (GLOW_LEDS - REAR_CENTER_OFFSET))
        sparkle_state.passBall = FRONT_CENTER_OFFSET

      sparkle_state.driverBall--
      sparkle_state.passBall++
      FastLED.show()
      break;
  }
  // wait before we do it again
  //
  // delay(2);
}
