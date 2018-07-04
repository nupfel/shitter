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

// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

uint8_t brightness = 255;
uint16_t fps = 120;
bool idle = true;
long fade_duration = 3000;
uint8_t pattern_duration_min = 3;
uint8_t pattern_duration_max = 15;
uint8_t pattern_duration = random8(pattern_duration_min, pattern_duration_max);
long last_run = 0;

void handle_serial();
void show();

// patterns
void idle_pattern();
void wave_up();
void wave_down();
void wave_left();
void wave_right();
void noise();
void Fire2012WithPalette();

bool gReverseDirection = false;

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { wave_up, wave_down, wave_left, wave_right, noise };
uint8_t gPatternIndex = random8(NUM_PATTERNS);

CRGB leds[NUM_LEDS];

CRGBPalette16 gPal;

void setup() {
        delay(1000); // sanity delay

        Serial.begin(115200);

        FastLED.addLeds<CHIPSET, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
        FastLED.setBrightness(brightness);

        // This first palette is the basic 'black body radiation' colors,
        // which run from black to red to bright yellow to white.
        gPal = HeatColors_p;

        // These are other ways to set up the color palette for the 'fire'.
        // First, a gradient from black to red to yellow to white -- similar to HeatColors_p
        // gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);

        // Second, this palette is like the heat colors, but blue/aqua instead of red/yellow
        // gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);

        // Third, here's a simpler, three-step gradient, from black to red to white
        // gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);

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

#ifdef DEBUG
        EVERY_N_SECONDS(1) {
                Serial.printf("[%d]\n", millis());
                Serial.printf("\tidle: %d\n", idle);
                Serial.printf("\tgPatternIndex: %d\n", gPatternIndex);
                Serial.printf("\tpattern_duration: %d\n", pattern_duration);
                Serial.printf("\tfps: %d\n", fps);
        }
#endif

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
        static uint8_t fade_interval = fade_duration/250;
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

        EVERY_N_SECONDS(pattern_duration) {
                // switch to random pattern
                gPatternIndex = random8(NUM_PATTERNS);

                // choose random length for next pattern
                pattern_duration = random8(pattern_duration_min, pattern_duration_max);

                Serial.printf("switching to pattern index %d for %ds\n", gPatternIndex, pattern_duration);
        }
}

void wave_up() {
        static uint8_t y = 0;
        static uint8_t colour_index = 0;

        colour_index = (255/(HEIGHT - 1)) * y;

#ifdef DEBUG
        Serial.printf("y: %d  c: %d\n", y, colour_index); //debug
#endif

        fadeToBlackBy(leds, NUM_LEDS, 10);

        for (uint8_t x = 0; x < WIDTH; x++) {
                leds[x*HEIGHT+y] = ColorFromPalette(RainbowColors_p, colour_index);
        }

        y = (y + 1) % HEIGHT;
}

void wave_down() {
        static uint8_t y = HEIGHT - 1;
        static uint8_t colour_index = 0;

        colour_index = (255/(HEIGHT - 1)) * y;

#ifdef DEBUG
        Serial.printf("y: %d  c: %d\n", y, colour_index); //debug
#endif

        fadeToBlackBy(leds, NUM_LEDS, 10);

        for (uint8_t x = 0; x < WIDTH; x++) {
                leds[x*HEIGHT+y] = ColorFromPalette(RainbowColors_p, colour_index);
        }

        y = (y - 1) % HEIGHT;
}

void wave_left() {
        static uint8_t x = 0;
        static uint8_t colour_index = 0;

        colour_index = (255/(WIDTH - 1)) * x;

#ifdef DEBUG
        Serial.printf("x: %d  c: %d\n", x, colour_index); //debug
#endif

        fadeToBlackBy(leds, NUM_LEDS, 10);

        for (uint8_t y = 0; y < HEIGHT; y++) {
                leds[x*HEIGHT+y] = ColorFromPalette(RainbowColors_p, colour_index);
        }

        x = (x + 1) % WIDTH;
}

void wave_right() {
        static uint8_t x = WIDTH - 1;
        static uint8_t colour_index = 0;

        colour_index = (255/(WIDTH - 1)) * x;

#ifdef DEBUG
        Serial.printf("x: %d  c: %d\n", x, colour_index); //debug
#endif

        fadeToBlackBy(leds, NUM_LEDS, 10);

        for (uint8_t y = 0; y < HEIGHT; y++) {
                leds[x*HEIGHT+y] = ColorFromPalette(RainbowColors_p, colour_index);
        }

        x = (x - 1) % WIDTH;
}

void noise() {
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
        CRGB purecolour = CHSV(hue,255,255);  // pure hue, full brightness
        CRGB lightcolour = CHSV(hue,200,255); // half 'whitened', full brightness
        gPal = CRGBPalette32(CRGB::Black, darkcolour, purecolour, lightcolour);

        // Fire2012WithPalette(); // run simulation frame, using palette colors

        fadeToBlackBy(leds, NUM_LEDS, 10);

        EVERY_N_MILLISECONDS(100) {
                for (uint8_t i = 0; i < 50; i++) {
                        leds[ random16(NUM_LEDS) ] = ColorFromPalette(gPal, random8());
                }
                // Serial.print("."); //debug
        }
}


// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
////
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation,
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
void Fire2012WithPalette()
{
// Array of temperature readings at each simulation cell
        static byte heat[NUM_LEDS];

        // Step 1.  Cool down every cell a little
        for( int i = 0; i < NUM_LEDS; i++) {
                heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
        }

        // Step 2.  Heat from each cell drifts 'up' and diffuses a little
        for( int k= NUM_LEDS - 1; k >= 2; k--) {
                heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
        }

        // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
        if( random8() < SPARKING ) {
                int y = random8(7);
                heat[y] = qadd8( heat[y], random8(160,255) );
        }

        // Step 4.  Map from heat cells to LED colors
        for( int j = 0; j < NUM_LEDS; j++) {
                // Scale the heat value from 0-255 down to 0-240
                // for best results with color palettes.
                byte colorindex = scale8( heat[j], 240);
                CRGB color = ColorFromPalette( gPal, colorindex);
                int pixelnumber;
                if( gReverseDirection ) {
                        pixelnumber = (NUM_LEDS-1) - j;
                } else {
                        pixelnumber = j;
                }
                leds[pixelnumber] = color;
        }
}
