// MonocLED.ino
//
// Controls 1 NeoPixel RGBW ring to display a variety of patterns.
//
// Alex Troesch (c) 2016

#include <Adafruit_NeoPixel.h>

// Output pin for the NeoPixel ring
#define PIXEL_PIN 0
// Type of NeoPixels connected
#define PIXEL_TYPE NEO_GRBW + NEO_KHZ800
// Number of pixels in the ring
#define PIXEL_COUNT 24
// Pixel brightness, 0-255
#define BRIGHTNESS_LOW 4
#define BRIGHTNESS_MED 16
#define BRIGHTNESS_HIGH 32

// Input pin for the mode cycle button
// Must be GPIO#2 for Trinket since that is the only external interupt INT0
#define BUTTON_PIN 2
// Button debounce time in milliseconds
// Prevents button from being triggered too frequently
#define DEBOUNCE_TIME 250

// Mode definitions
#define MODE_OFF 0
#define MODE_COMET 1
#define MODE_BOUNCE 2
#define MODE_HIPPY 3
#define MODE_MELT 4
#define MODE_COUNT 5

// Animation patterns
enum Pattern {
  NONE,
  COLOR_WIPE,
  SCANNER,
  SLIDE,
  RAINBOW_CYCLE,
  THEATER_CHASE
};

// NeoPattern class
// Manages a Neopixel array with a simple animation loop.
//
// Must provide the number of pixels, output pin, and type of neopixels as
// defined in <Adafruit_NeoPixel.h>. Each Pattern value determines the animation
// loop and provides 3 parameters via the setPattern function. The interval
// controls the milliseconds between updates. The total_steps controls the total
// animation steps in the loop, this may need to be equal to the number of
// controled pixels for some loops and for others it will control the overall
// length of the animation given by interval * total_steps (in ms). Finally, the
// color parameter will set an additional background color for some modes.
//
// To add a new pattern you must update the Pattern enum, the update function,
// as well as add a new function to execute the pattern, usually called
// patternNameUpdate. There a few utility functions defined in the NeoPatterns
// class to assist in color calcuations: dimColor, red, green, blue, and wheel.
//
// Usage Example:
// 
// NeoPatterns ring(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
// void setup() {
//   ring.begin();
//   ring.setBrightness(127);
//   ring.setPattern(PATTERN, 60, 100, ring.Color(255, 0, 0));
// }
// void loop() {
//   ring.run();
// }
class NeoPatterns : public Adafruit_NeoPixel {
  public:
  NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type)
    : Adafruit_NeoPixel(pixels, pin, type) {}

  // Defines main animation loop
  void run() {
    // Delay until at least interval_ ms have elapsed
    if ((millis() - last_update_) < interval_) return;
    last_update_ = millis();
    // Compute next set of animation
    update(active_pattern_);
    // Pushes pixel data to the ring
    show();
    // Advance to next animatation state
    index_ = ++index_ % total_steps_;
  }

  // Update animation settings
  void setPattern(Pattern pattern, uint8_t interval, uint16_t total_steps,
                  uint32_t color) {
    active_pattern_ = pattern;
    interval_ = interval;
    total_steps_ = total_steps;
    color_ = color;
    index_ = 0;
  }

  private:
  void update(Pattern active) {
    switch(active) {
      case COLOR_WIPE:
        colorWipeUpdate();
        break;
      case SCANNER:
        scannerUpdate();
        break;
      case SLIDE:
        slideUpdate();
        break;
      case RAINBOW_CYCLE:
        rainbowCycleUpdate();
        break;
      case THEATER_CHASE:
        theaterChaseUpdate();
        break;
    }
  }

  void colorWipeUpdate() {
    setPixelColor(index_, color_);
  }

  void scannerUpdate() { 
    for (int i = 0; i < PIXEL_COUNT; ++i) {
      setPixelColor(i, dimColor(getPixelColor(i)));
      if (i == index_) {
        setPixelColor(i, wheel(((i * 256 / PIXEL_COUNT) + index_) & 255));
      }
    }
  }

  void slideUpdate() {
    uint32_t color = wheel((index_ * 256 / (3 * PIXEL_COUNT)) & 255);
    for (int i = 0; i < PIXEL_COUNT; ++i) {
      setPixelColor(i, Color(0,0,0));
      if (i == index_ % PIXEL_COUNT ||
          i == (3 * PIXEL_COUNT - index_) % PIXEL_COUNT) {
        setPixelColor(i, color);
      }
    }
  }

  void rainbowCycleUpdate() {
    for (int i = 0; i < PIXEL_COUNT; ++i) {
      setPixelColor(i, wheel(((i * 256 / PIXEL_COUNT) + index_) & 255));
    }
  }

  void theaterChaseUpdate() {
    for(int i = 0; i < PIXEL_COUNT; ++i) {
      if ((i + index_) % 3 == 0) {
        setPixelColor(i, color_);
      } else {
        setPixelColor(i, wheel(index_ & 255));
      }
    }
  }

  // Calculate 50% dimmed version of a color
  uint32_t dimColor(uint32_t color) {
    uint32_t dimColor = Color(red(color) >> 1,
                              green(color) >> 1,
                              blue(color) >> 1);
    return dimColor;
  }

  // Returns the red component of a 32-bit color
  uint8_t red(uint32_t color) {
    return (color >> 16) & 0xFF;
  }

  // Returns the green component of a 32-bit color
  uint8_t green(uint32_t color) {
    return (color >> 8) & 0xFF;
  }

  // Returns the blue component of a 32-bit color
  uint8_t blue(uint32_t color) {
    return color & 0xFF;
  }
  
  // Input a value 0 to 255 to get a color value.
  // The colors are a transition r -> g -> b -> r.
  uint32_t wheel(byte wheel_pos) {
    wheel_pos = 255 - wheel_pos;

    if (wheel_pos < 85) {
      return Color(255 - wheel_pos * 3, 0, wheel_pos * 3);
    } else if (wheel_pos < 170) {
      wheel_pos -= 85;
      return Color(0, wheel_pos * 3, 255 - wheel_pos * 3);
    } else {
      wheel_pos -= 170;
      return Color(wheel_pos * 3, 255 - wheel_pos * 3, 0);
    }
  }
  
  Pattern active_pattern_;  // Which pattern is running
  unsigned long interval_;  // Milliseconds between pixel updates
  unsigned long last_update_;  // Time of last update in milliseconds
  
  uint32_t color_;  // What color is in use
  uint16_t total_steps_;  // Total number of steps in the pattern
  uint16_t index_;  // Current step within the pattern
};

// Setup
NeoPatterns ring(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// Initialize ring state
// Must be volatile as the state is modified by interupts
volatile uint8_t display_mode = MODE_COMET;
volatile unsigned long last_press = 0;

// Called on INT0 HIGH -> LOW
void buttonHandler() {
    if (millis() - last_press > DEBOUNCE_TIME) {
        last_press = millis();
        display_mode = ++display_mode % MODE_COUNT;
        switchMode(display_mode);
    }
}

// Switch between mode presets
void switchMode(uint8_t mode) {
  switch (mode) {
    case MODE_OFF:
      ring.setPattern(COLOR_WIPE, 75, PIXEL_COUNT, 0);
      break;
    case MODE_COMET:
      ring.setBrightness(BRIGHTNESS_HIGH);
      ring.setPattern(SCANNER, 65, PIXEL_COUNT, 0);
      break;
    case MODE_BOUNCE:
      ring.setPattern(SLIDE, 65, PIXEL_COUNT, 0);
      break;
    case MODE_HIPPY:
      ring.setBrightness(BRIGHTNESS_LOW);
      ring.setPattern(RAINBOW_CYCLE, 3, 255, 0);
      break;
    case MODE_MELT:
      ring.setPattern(THEATER_CHASE, 65, 255, ring.Color(0, 127, 255));
      break;
  }
}

// Initial setup, runs once upon boot
void setup() {
    // Set the button pin to input and fire an interupt
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(0, buttonHandler, FALLING);

    ring.begin();
    switchMode(display_mode);
}

// Main loop, runs repeatedly
void loop() {
    ring.run();
}
