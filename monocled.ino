/*
 * MonocLED
 *
 * Controls 1 NeoPixel RGBW ring to display a variety of patterns with
 * a debounced button to cycle display modes. Hold this button to turn
 * off the ring.
 *
 * Alex Troesch (c) 2016
 */

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

// Output pin for the NeoPixel ring
#define PIXEL_PIN 1

// Number of pixels in the ring
#define PIXEL_COUNT 24

// Pixel brightness, 0-255
#define BRIGHTNESS 127

// Input pin for the mode cycle button
#define BUTTON_PIN 2

// Button timing parameters in ms
#define DEBOUNCE_TIME 100
#define HOLD_TIME 2000

// Mode definitions
#define MODE_OFF   0
#define MODE_SWIRL 1
#define MODE_STARS 2
#define MODE_PULSE 3
#define MODE_THUMP 4
#define MODE_HIPPY 5
#define MODE_COUNT 6

// Initialize display state
uint8_t displayMode = MODE_OFF;

// Initialize button state
// Due to internal pull-up resistors
// LOW = pressed, HIGH = not pressed
bool previousState = HIGH;
bool currentState;
long millis_held;
unsigned long startTime;

// Create the ring
NeoPixelBus<NeoRgbwFeature, Neo800KbpsMethod> ring(PIXEL_COUNT, PIXEL_PIN); 

// Create animation controller with millisecond scale updates
NeoPixelAnimator animator(1, NEO_MILLISECONDS);

// Initial setup, runs once upon boot
void setup() {
    // Set the button pin to input
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    startTime = millis();

    // Initialize to off
    displayMode = MODE_OFF;
    ring.Begin();
    ring.Show();
}

// Main loop, runs repeatedly
void loop() {
    // Read button state
    currentState = digitalRead(BUTTON_PIN);

    // Update the start time of a button press at least 200ms
    // after the previous time of a HIGH -> LOW transition
    if (currentState == LOW && previousState == HIGH
            && (millis() - startTime > 2 * DEBOUNCE_TIME)) {
        startTime = millis();
    }

    millis_held = millis() - startTime;

    // Debounce, button is required to be pressed for 100ms
    if (millis_held > DEBOUNCE_TIME && millis_held < HOLD_TIME) {

        // Switch mode on button release
        if (currentState == HIGH && previousState == LOW) {

            // Increment mode
            // Looping back to mode 1 instead of off
            displayMode++;
            if (displayMode >= MODE_COUNT) {
                displayMode = 1;
            }

            // Start new animation
            switchMode(displayMode);
        }

    // Switch ring to MODE_OFF if held for more than 2s
    } else if (millis_held >= HOLD_TIME) {
        displayMode = MODE_OFF;
        switchMode(displayMode);
    }

    // Update animation and display
    animator.UpdateAnimations();
    if (ring.IsDirty() && ring.CanShow()) {
        ring.Show();
    }

    // Remember the current state as the previous state
    previousState = currentState;
}

/* 
 * AnimationParam
 *
 * This struct contains three properties that provide state information to the animation callback.
 *
 * float progress - the progress from 0.0 to 1.0 of the animation to apply
 * uint16_t index - the channel index of the animation
 * AnimationState state - the animation state, which can be one of the following...
 *   AnimationState_Started - this is the first call to update, will only be set once
 *     unless the animation is restarted.
 *   AnimationState_Progress - this is one of the many calls between the first and last.
 *   AnimationState_Completed - this is the last call to update, will only be set once
 *     unless the animation is restarted.
 *
 * Easing Functions
 *
 * NeoEase::Linear
 * NeoEase::QuadraticInOut
 * NeoEase::CubicInOut
 * NeoEase::QuarticInOut
 * NeoEase::QuinticInOut
 * NeoEase::SinusoidalInOut
 * NeoEase::ExponentialInOut
 * NeoEase::CircularInOut
 *
 */

/*
 * MODE_OFF
 *
 * Fades each pixel to black in order. Does not repeat.
 * 
 */
void offUpdate(const AnimationParam& param) {
    uint8_t endPixel = param.progress * PIXEL_COUNT;
    RgbwColor color;

    if (endPixel > PIXEL_COUNT) {
        endPixel = PIXEL_COUNT;
    }

    for (uint8_t i = 0; i <= endPixel; i++) {
        color = RgbwColor::LinearBlend(ring.GetPixelColor(i), RgbwColor(0,0,0,0), param.progress);
        ring.SetPixelColor(i, color);
    }
}

/*
 * MODE_SWIRL
 *
 * A colorful comet swirls in one direction.
 * 
 */
struct SwirlParam {
    uint8_t lastPixel;
} swirlParam;
void swirlUpdate(const AnimationParam& param) {
    uint8_t nextPixel = param.progress * PIXEL_COUNT;

    fadeAll(10);

    if (nextPixel != swirlParam.lastPixel) {
        for (uint8_t i =  swirlParam.lastPixel + 1; i <= nextPixel; i++) {
            ring.SetPixelColor(i, RgbwColor(0, 153, 255, 0));
        }
    }

    swirlParam.lastPixel = nextPixel;

    // Restart if compeleted
    if (param.state == AnimationState_Completed) {
        // Adjust Parameters for next run if needed
        swirlParam.lastPixel = 0;

        animator.RestartAnimation(param.index);
    }
}

/*
 * MODE_STARS
 * 
 */
struct StarsParam {
} starsParam;
void starsUpdate(const AnimationParam& param) {

    // Restart if compeleted
    if (param.state == AnimationState_Completed) {
        // Adjust Parameters for next run if needed
        animator.RestartAnimation(param.index);
    }
}

/*
 * MODE_PULSE
 *
 */
struct PulseParam {
} pulseParam;
void pulseUpdate(const AnimationParam& param) {

    // Restart if compeleted
    if (param.state == AnimationState_Completed) {
        // Adjust Parameters for next run if needed
        animator.RestartAnimation(param.index);
    }
}

/*
 * MODE_THUMP
 *
 */
struct ThumpParam {
} thumpParam;
void thumpUpdate(const AnimationParam& param) {

    // Restart if compeleted
    if (param.state == AnimationState_Completed) {
        // Adjust Parameters for next run if needed
        animator.RestartAnimation(param.index);
    }
}

/*
 * MODE_HIPPY
 *
 */
struct HippyParam {
} hippyParam;
void hippyUpdate(const AnimationParam& param) {

    // Restart if compeleted
    if (param.state == AnimationState_Completed) {
        // Adjust Parameters for next run if needed
        animator.RestartAnimation(param.index);
    }
}

// Fade out the whole ring
void fadeAll(uint8_t darkenBy) {
    RgbwColor color;

    for (uint8_t i = 0; i < PIXEL_COUNT; i++) {
        color = ring.GetPixelColor(i);
        color.Darken(darkenBy);
        ring.SetPixelColor(i, color);
    }
}

// Create animations and set parameters
void switchMode(uint8_t mode) {
    switch (mode) {
        case MODE_OFF:
            animator.StartAnimation(0, 2000, offUpdate);
            break;
        case MODE_SWIRL:
            swirlParam.lastPixel = 0;
            animator.StartAnimation(0, 5000, swirlUpdate);
            break;
        case MODE_STARS:
            animator.StartAnimation(0, 5000, starsUpdate);
            break;
        case MODE_PULSE:
            animator.StartAnimation(0, 5000, pulseUpdate);
            break;
        case MODE_THUMP:
            animator.StartAnimation(0, 5000, thumpUpdate);
            break;
        case MODE_HIPPY:
            animator.StartAnimation(0, 5000, hippyUpdate);
            break;
    }
}
