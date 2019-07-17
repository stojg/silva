// lipo protection
// https://www.youtube.com/watch?v=1Fs4SfVSsLk

// tips and tricks
// https://github.com/atuline/FastLED-Demos/blob/master/fastled_tips_snippets.h

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

#define LED_PIN     3
#define COLOR_ORDER GRB
#define CHIPSET     WS2812B

//#define NUM_LEDS 23
#define NUM_LEDS 55

#define FRAMES_PER_SECOND 60

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#include "Arduino.h"
#include "FastLED.h"

CRGB leds[NUM_LEDS];

// the max brightness is set as a global variable so it can be changed by pots or other events
uint8_t max_bright = 64;

// Index number of which pattern is current
uint8_t gCurrentPatternNumber = 0;

// rotating "base color" used by many of the patterns
uint8_t gHue = 0;

void setup() {
    // 1 second delay for recovery in case there is something crashing the board and we need to reflash it
    delay(1000);

    // tell FastLED about the LED strip configuration
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

    // set master brightness control
    FastLED.setBrightness(max_bright);

    // be careful about how much amperage we use
    set_max_power_in_volts_and_milliamps(5, 100);

    // this pin will be set high when the FASTLED is limiting 'power'
    set_max_power_indicator_LED(LED_BUILTIN);

    // seed the different randoms
    random16_set_seed(4832);
    random16_add_entropy(analogRead(2));
}

void rainbow() {
    // FastLED's built-in rainbow generator
    fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void addGlitter(fract8 chanceOfGlitter) {
    if (random8() < chanceOfGlitter) {
        leds[random16(NUM_LEDS)] += CRGB::White;
    }
}

void rainbowWithGlitter() {
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter(10);
}

void confetti() {
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy(leds, NUM_LEDS, 2);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon() {
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy(leds, NUM_LEDS, 20);
    int pos = beatsin16(10, 0, NUM_LEDS - 1);
    leds[pos] += CHSV(gHue, 255, 192);
}

void bpm() {
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    for (int i = 0; i < NUM_LEDS; i++) { //9948
        leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }
}

void juggle() {
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(leds, NUM_LEDS, 20);
    byte dothue = 0;
    for (int i = 0; i < 3; i++) {
        leds[beatsin16(i + 9, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}

void meteor() {
    static uint8_t pos = 0;
    const uint8_t size = 3;

    if (pos > NUM_LEDS * 2) {
        pos = 0;
    }

    for (auto &led : leds) {
        if ((random8(10) > 5)) {
            led.fadeToBlackBy(96);
        }
    }
    if (pos < NUM_LEDS) {
        for (uint8_t i = 0; i < size; i++) {
            leds[pos + i] = ColorFromPalette(PartyColors_p, gHue, 200);
        }
    }
    pos += 1;
}

// List of patterns to cycle through.
typedef void (*SimplePatternList[])();

SimplePatternList gPatterns = {meteor, sinelon, juggle, bpm};

// Change pattern
void nextPattern() {
    // add one to the current pattern number, and wrap around at the end
    for (uint8_t i = 0; i < FRAMES_PER_SECOND / 4; i++) {
        fadeToBlackBy(leds, NUM_LEDS, 32);
        FastLED.show();
        FastLED.delay(1000 / FRAMES_PER_SECOND);
    }

    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
}

void loop() {
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    // insert a delay to keep the framerate modest
    FastLED.delay(1000 / FRAMES_PER_SECOND);

    // do some periodic updates
    EVERY_N_MILLISECONDS(20) { gHue++; } // slowly cycle the "base color" through the rainbow
    EVERY_N_SECONDS(30) { nextPattern(); } // change patterns periodically
}
