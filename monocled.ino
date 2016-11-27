/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * MonocLED
 *
 * Controls 1 NeoPixel RGBW ring to display a variety of patterns with
 * a debounced button to cycle display modes.
 *
 * Alex Troesch (c) 2016
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#include <Adafruit_NeoPixel.h>

// Output pin for the NeoPixel ring
#define PIXEL_PIN 0

// Number of pixels in the ring
#define PIXEL_COUNT 24

// Pixel brightness, 0-255
#define BRIGHTNESS_LOW 8
#define BRIGHTNESS_HIGH 64

// Input pin for the mode cycle button
// Must be GPIO#2 for Trinket since that is the only external interupt INT0
#define BUTTON_PIN 2

// Button timing parameters in ms
#define DEBOUNCE_TIME 100

// Mode definitions
#define MODE_OFF   0

#define MODE_SWIRL 1
#define MODE_SLIDE 2
#define MODE_SPARK 3
#define MODE_THUMP 4
#define MODE_HIPPY 5

#define MODE_COUNT 6

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * NeoPatterns
 *
 * Derived from Adafruit example. Allows for pixel buffer to be updated in a non-blocking way.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE, SLIDE };

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel {
    public:

    // Member Variables:  
    pattern ActivePattern;  // which pattern is running
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type)
    :Adafruit_NeoPixel(pixels, pin, type) { }
    
    // Update the pattern
    void Update() {
        // time to update
        if ((millis() - lastUpdate) > Interval) {
            lastUpdate = millis();

            switch(ActivePattern) {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                case SLIDE:
                    SlideUpdate();
                    break;
                default:
                    break;
            }
            
            show();
            Increment();
        }
    }
  
    // Increment the Index and reset at the end
    void Increment() {
        Index++;
        if (Index >= TotalSteps) {
            Index = 0;
        }
    }
    
    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval) {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate() {
        for (int i = 0; i< PIXEL_COUNT; i++) {
            setPixelColor(i, Wheel(((i * 256 / PIXEL_COUNT) + Index) & 255));
        }
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval) {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = PIXEL_COUNT;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
   }
    
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate() {
        for(int i=0; i< PIXEL_COUNT; i++) {
            if ((i + Index) % 3 == 0) {
                setPixelColor(i, Color1);
            } else {
                setPixelColor(i, Color2);
            }
        }
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval) {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = PIXEL_COUNT;
        Color1 = color;
        Index = 0;
    }
    
    // Update the Color Wipe Pattern
    void ColorWipeUpdate() {
        setPixelColor(Index, Color1);
    }
    
    // Initialize for a SCANNNER
    void Scanner(uint8_t interval) {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = PIXEL_COUNT;
        Index = 0;
    }

    // Update the Scanner Pattern
    void ScannerUpdate() { 
        for (int i = 0; i < PIXEL_COUNT; i++) {
            if (i == Index) {  // Scan Pixel to the right
                setPixelColor(i, Wheel(((i * 256 / PIXEL_COUNT) + Index) & 255));
            } else { // Fading tail
                setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
    }

    // Initialize for a SLIDE
    void Slide(uint8_t interval) {
        ActivePattern = SLIDE;
        Interval = interval;
        TotalSteps = 3 * PIXEL_COUNT;
        Index = 0;
    }

    // Update the Scanner Pattern
    void SlideUpdate() {
        uint32_t color = Wheel((Index * 256 / (3 * PIXEL_COUNT)) & 255);
        for (int i = 0; i < PIXEL_COUNT; i++) {
            if (i == Index % PIXEL_COUNT) {
                setPixelColor(i, color);
            } else if (i == (3 * PIXEL_COUNT - Index) % PIXEL_COUNT) {
                setPixelColor(i, color);
            } else {
                setPixelColor(i, Color(0,0,0));
            }
        }
    }
    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval) {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
    }
    
    // Update the Fade Pattern
    void FadeUpdate() {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        
        for (int i = 0; i < PIXEL_COUNT; i++) {
            setPixelColor(i, Color(red, green, blue));
        }
    }
   
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color) {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color) {
        for (int i = 0; i < PIXEL_COUNT; i++) {
            setPixelColor(i, color);
        }
        show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color) {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color) {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color) {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos) {
        WheelPos = 255 - WheelPos;

        if (WheelPos < 85) {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        } else if (WheelPos < 170) {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        } else {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * setup, loop, and button logic
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

// Create the ring
// Adafruit_NeoPixel ring(PIXEL_COUNT, PIXEL_PIN, NEO_GRBW + NEO_KHZ800);
NeoPatterns ring(PIXEL_COUNT, PIXEL_PIN, NEO_GRBW + NEO_KHZ800);

// Initialize display state
volatile uint8_t displayMode = 1;

// Initialize button debounce timer
volatile unsigned long lastPress = 0;

// Called on INT0 HIGH -> LOW, stores button state
void buttonHandler() {
    if (millis() - lastPress > DEBOUNCE_TIME) {
        if (++displayMode >= MODE_COUNT) {
            displayMode = 0;
        }
        switchMode(displayMode);
    }

    lastPress = millis();
}

// Initial setup, runs once upon boot
void setup() {
    // Set the button pin to input and fire an interupt
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(0, buttonHandler, FALLING);

    // Initialize to first mode
    ring.setBrightness(BRIGHTNESS_LOW);
    ring.begin();
    ring.show();
    switchMode(displayMode);
}

// Main loop, runs repeatedly
void loop() {
    // Update animation and display
    ring.Update();
}

// Create animations and set parameters
void switchMode(uint8_t mode) {
    byte color = random(255);
    uint32_t randomColor1 = ring.Wheel(color);
    uint32_t randomColor2 = ring.Wheel((color + 128) % 256);

    switch (mode) {
        case MODE_OFF:
            ring.ColorWipe(ring.Color(0, 0, 0), 75);
            break;
        case MODE_SWIRL:
            ring.setBrightness(BRIGHTNESS_HIGH);
            ring.Scanner(65);
            break;
        case MODE_SLIDE:
            ring.Slide(65);
            break;
        case MODE_SPARK:
            ring.setBrightness(BRIGHTNESS_LOW);
            ring.TheaterChase(randomColor1, ring.Color(0,0,0), 100);
            break;
        case MODE_THUMP:
            ring.Fade(randomColor1, randomColor2, 90, 10);
            break;
        case MODE_HIPPY:
            ring.RainbowCycle(3);
            break;
    }
}
