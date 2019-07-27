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
uint8_t gPatternIndex = 0;

// rotating "base color" used by many of the patterns
uint8_t gHue = 0;

uint8_t gBlend = 0;

#define CURRENT_SCRATCH 0
#define PREV_SCRATCH 1
CRGB scratch[2][NUM_LEDS];

void setup() {
    // 1 second delay for recovery in case there is something crashing the board and we need to re-flash it
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

    FastLED.clear(true);
//    for (uint8_t i = 0; i < 3; i++) {
//        leds[0] = CRGB::White;
//        FastLED.show();
//        delay(300);
//        leds[0] = CRGB::Black;
//        delay(300);
//        FastLED.show();
//    }
//    FastLED.delay((1000);
}

void rainbow(CRGB *in) {
    // FastLED's built-in rainbow generator
    fill_rainbow(in, NUM_LEDS, gHue, 7);
}

void addGlitter(fract8 chanceOfGlitter, CRGB *in) {
    if (random8() < chanceOfGlitter) {
        in[random16(NUM_LEDS)] += CRGB::White;
    }
}

void rainbowWithGlitter(CRGB *in) {
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow(in);
    addGlitter(10, in);
}

void confetti(CRGB *in) {
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy(in, NUM_LEDS, 2);
    int pos = random16(NUM_LEDS);
    in[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon(CRGB *in) {
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy(in, NUM_LEDS, 20);
    int pos = beatsin16(10, 0, NUM_LEDS - 1);
    in[pos] += CHSV(gHue, 255, 192);
}

void bpm(CRGB *in) {
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    for (int i = 0; i < NUM_LEDS; i++) { //9948
        in[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }
}

void juggle(CRGB *in) {
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(in, NUM_LEDS, 20);
    byte dothue = 0;
    for (int i = 0; i < 3; i++) {
        in[beatsin16(i + 9, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}

void meteor(CRGB *in) {
    static uint8_t pos = 0;
    const uint8_t size = 2;

    if (pos > NUM_LEDS * 2) {
        pos = 0;
    }

    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        if ((random8(10) > 5)) {
            in[i].fadeToBlackBy(96);
        }
    }

    pos += 1;

    // we are outside the led strip, so do not paint anymore
    if (pos >= NUM_LEDS) {
        return;
    }

    // paint the meteor
    for (uint8_t i = 0; i < size; i++) {
        in[pos - 1] = ColorFromPalette(PartyColors_p, gHue, 200);
    }
}



void blue(CRGB *in) {
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        in[i].setHSV(160, 255, 128);
    }
}

void green(CRGB *in) {
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        in[i].setHSV(96, 255, 128);
    }
}

void red(CRGB *in) {
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        in[i].setHSV(0, 255, 128);
    }
}

// List of patterns to cycle through.
typedef void (*SimplePatternList[])(CRGB *in);
SimplePatternList gPatterns = {meteor, sinelon, juggle, bpm};
//SimplePatternList gPatterns = {meteor, juggle};
//SimplePatternList gPatterns = {red, green, blue};
//SimplePatternList gPatterns = {red, green, blue};

uint8_t nextPatternIndex() {
    return (gPatternIndex + 1) % ARRAY_SIZE(gPatterns);
}

uint8_t prevPatternIndex() {
    if (gPatternIndex == 0) {
        return ARRAY_SIZE(gPatterns)-1;
    }

    return (gPatternIndex - 1) % ARRAY_SIZE(gPatterns);
}

void update(uint8_t blending) {
    gPatterns[gPatternIndex](scratch[CURRENT_SCRATCH]);
    // we are transitioning, so keep updating the old pattern
    if (blending != 0) {
        gPatterns[prevPatternIndex()](scratch[PREV_SCRATCH]);
    }
}

// Copy the leds from the scratch spaces with index into the output display leds
void blit(uint8_t blending) {
    // might be faster, might not depending on the conditional
    if (blending == 0) {
        memmove8(&leds, &scratch[CURRENT_SCRATCH], NUM_LEDS * sizeof(CRGB));
    } else {
        for (uint8_t i = 0; i < NUM_LEDS; i++) {
            // make sure to use blend for CHSV and nblend for CRGB
            leds[i] = nblend(scratch[CURRENT_SCRATCH][i], scratch[PREV_SCRATCH][i], 255-blending);
        }
    }
}

bool transition = false;

// Change pattern
void nextPattern() {
    memmove8(&scratch[PREV_SCRATCH], &scratch[CURRENT_SCRATCH], NUM_LEDS * sizeof(CRGB));
    gPatternIndex = nextPatternIndex();
    transition = true;
}

void loop() {
    EVERY_N_MILLISECONDS(1000 / FRAMES_PER_SECOND) {
        if (transition) {
            if (gBlend > 254) {
                transition = false;
                gBlend = 0;
                fill_solid(scratch[PREV_SCRATCH], NUM_LEDS, CHSV(0,0,0));
            } else {
                gBlend += 2;
            }
        }

        update(gBlend);
        blit(gBlend);
        leds->setHSV(0,0,gBlend);
        FastLED.show();
    }

    // slowly cycle the "base color" through the rainbow
    EVERY_N_MILLISECONDS(20) { gHue++; }
    EVERY_N_SECONDS(15) { nextPattern(); } // change patterns periodically
}
