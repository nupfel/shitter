#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_INTERNAL
#define FASTLED_ALLOW_INTERRUPTS 0

#include <FastLED.h>

// #define DEBUG

#define LED_PIN     D1
#define CHIPSET     WS2812B
#define WIDTH       30
#define HEIGHT      37
#define NUM_LEDS    1110
#define NUM_PATTERNS 4

uint8_t brightness = 255;
uint16_t fps = 160;
bool idle = true;
long fade_duration = 4000;
uint8_t pattern_duration_min = 2;
uint8_t pattern_duration_max = 7;
uint8_t pattern_duration = random8(pattern_duration_min, pattern_duration_max);
uint8_t glitter_duration_min = 1;
uint8_t glitter_duration_max = 5;
long last_run = 0;
bool add_glitter = false;

void handle_serial();
void show();

// patterns
void idle_pattern();
void wave_up();
void wave_down();
void wave_left();
void wave_right();
void glitter();
void Fire2012WithPalette();

bool gReverseDirection = false;

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { wave_up, wave_down, wave_left, wave_right };
uint8_t gPatternIndex = random8(NUM_PATTERNS);

CRGB leds[NUM_LEDS];

CRGBPalette16 gPal;

void setup() {
        delay(1000); // sanity delay

        Serial.begin(115200);

        FastLED.addLeds<CHIPSET, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
        FastLED.setBrightness(brightness);

#ifdef DEBUG
        Serial.println();
        Serial.println(HEIGHT);
        Serial.println(WIDTH);
        Serial.println(NUM_LEDS);
#endif
        Serial.println("OK");
}

void loop()
{
        handle_serial();

        EVERY_N_SECONDS(1) {
                Serial.printf("[%d]\n", millis());
                Serial.printf("\tidle: %d\n", idle);
                // Serial.printf("\tfps: %d\n", fps);
                if (!idle) {
                        Serial.printf("\tgPatternIndex: %d\n", gPatternIndex);
                        Serial.printf("\tpattern_duration: %d\n", pattern_duration);
                }
        }

        if (idle == true) {
                idle_pattern();
        }
        else {
                show();
        }

        FastLED.show();
        FastLED.delay(1000 / fps);
}

void handle_serial() {
        uint8_t cmd;

        if (Serial.available()) {
                cmd = Serial.read();

                switch (cmd) {
                case 0:
                        idle = true;
                        Serial.println("idle mode");
                        break;
                case 1:
                        idle = false;
                        last_run = millis();
                        gPatternIndex = random8(NUM_PATTERNS);
                        pattern_duration = random8(pattern_duration_min, pattern_duration_max);
                        Serial.println("show time");
                        break;
                default:
                        Serial.printf("unkown command: %d\n", cmd);
                        idle = true;
                }

                // flish the rest if there is any
                Serial.flush();
        }
}

void show() {
        static uint8_t fade_interval = (fade_duration - 1000)/255;
        long now = millis();

        // first fade to black
        if (now - last_run < fade_duration) {
                EVERY_N_MILLISECONDS(fade_interval) {
                        // Serial.printf("[%d] fade\n", now); //debug
                        fadeToBlackBy(leds, NUM_LEDS, 1);
                }
                return;
        }

        gPatterns[gPatternIndex]();
        glitter();
        
        EVERY_N_SECONDS_I(glitter_timer, random8(glitter_duration_min, glitter_duration_max)) {
                add_glitter = !add_glitter;
                glitter_timer.setPeriod(random8(glitter_duration_min, glitter_duration_max));
        }

        EVERY_N_SECONDS_I(timer, pattern_duration) {
                // switch to random pattern
                gPatternIndex = random8(NUM_PATTERNS);

                // choose random length for next pattern
                pattern_duration = random8(pattern_duration_min, pattern_duration_max);
                timer.setPeriod(pattern_duration);

                Serial.printf("switching to pattern index %d for %ds\n", gPatternIndex, pattern_duration);
        }
}

void wave_up() {
        static uint8_t y = 0;
        static uint8_t colour_index = 0;
        static uint8_t base_hue = 0;

        colour_index = base_hue + 2 * y;

#ifdef DEBUG
        Serial.printf("y: %d  c: %d\n", y, colour_index); //debug
#endif

        fadeToBlackBy(leds, NUM_LEDS, 255/(HEIGHT/4));

        for (uint8_t x = 0; x < WIDTH; x++) {
                if (x % 2) {
                        leds[x * HEIGHT + y] = ColorFromPalette(RainbowColors_p, colour_index);
                }
                else {
                        leds[((x + 1) * HEIGHT) - 1 - y] = ColorFromPalette(RainbowColors_p, colour_index);
                }
        }

        y = (y + 1) % HEIGHT;
        base_hue++;
}

void wave_down() {
        static uint8_t y = HEIGHT - 1;
        static uint8_t colour_index = 0;
        static uint8_t base_hue = 0;

        colour_index = base_hue + 2 * y;

#ifdef DEBUG
        Serial.printf("y: %d  c: %d\n", y, colour_index); //debug
#endif

        fadeToBlackBy(leds, NUM_LEDS, 255/(HEIGHT/4));

        for (uint8_t x = 0; x < WIDTH; x++) {
                if (x % 2) {
                        leds[x * HEIGHT + y] = ColorFromPalette(RainbowColors_p, colour_index);
                }
                else {
                        leds[((x + 1) * HEIGHT) - 1 - y] = ColorFromPalette(RainbowColors_p, colour_index);
                }
        }

        y = (y - 1) % HEIGHT;
        base_hue++;
}

void wave_left() {
        static uint8_t x = 0;
        static uint8_t colour_index = 0;
        static uint8_t base_hue = 0;

        colour_index = base_hue + 2 * x;

#ifdef DEBUG
        Serial.printf("x: %d  c: %d\n", x, colour_index); //debug
#endif

        fadeToBlackBy(leds, NUM_LEDS, 255/(WIDTH/4));

        for (uint8_t y = 0; y < HEIGHT; y++) {
                leds[x * HEIGHT + y] = ColorFromPalette(RainbowColors_p, colour_index);
        }

        x = (x + 1) % WIDTH;
        base_hue++;
}

void wave_right() {
        static uint8_t x = WIDTH - 1;
        static uint8_t colour_index = 0;
        static uint8_t base_hue = 0;

        colour_index = base_hue + 2 * x;

#ifdef DEBUG
        Serial.printf("x: %d  c: %d\n", x, colour_index); //debug
#endif

        fadeToBlackBy(leds, NUM_LEDS, 255/(WIDTH/4));

        for (uint8_t y = 0; y < HEIGHT; y++) {
                leds[x * HEIGHT + y] = ColorFromPalette(RainbowColors_p, colour_index);
        }

        x = (x - 1) % WIDTH;
        base_hue++;
}

void glitter() {
        if (add_glitter) {
                for (uint8_t i = 0; i < 20; i++) {
                        leds[random16(NUM_LEDS)] += CRGB::White;
                }
        }
}

void idle_pattern() {
        // Fourth, the most sophisticated: this one sets up a new palette every
        // time through the loop, based on a hue that changes every time.
        // The palette is a gradient from black, to a dark color based on the hue,
        // to a light color based on the hue, to white.
        //
        static uint8_t hue = 0;
        hue++;
        CRGB darkcolour  = CHSV(hue,255,192); // pure hue, 3/4 brightness
        CRGB purecolour  = CHSV(hue,255,255); // pure hue, full brightness
        CRGB lightcolour = CHSV(hue,200,255); // half 'whitened', full brightness
        gPal = CRGBPalette32(CRGB::Black, darkcolour, purecolour, lightcolour);

        // Fire2012WithPalette(); // run simulation frame, using palette colors

        fadeToBlackBy(leds, NUM_LEDS, 20);

        EVERY_N_MILLISECONDS(100) {
                for (uint8_t i = 0; i < 50; i++) {
                        leds[ random16(NUM_LEDS) ] = ColorFromPalette(gPal, random8());
                }
                // Serial.print("."); //debug
        }
}
