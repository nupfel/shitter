#ifdef ESP8266
#include <NeoPixelBus.h>
#else
#include <OctoWS2811.h>
#endif

#include <FastLED.h>

// #define DEBUG

const uint8_t height = 37;
const uint8_t width = 30;

#define NUM_LEDS    (height * width)
#define NUM_PATTERNS 4

#ifdef ESP8266
NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> esp_leds(NUM_LEDS);
#else
DMAMEM int displayMemory[NUM_LEDS * 6];
int drawingMemory[NUM_LEDS * 6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 octo_leds(NUM_LEDS, displayMemory, drawingMemory, config);
#endif

uint8_t brightness = 255;
uint8_t saturation = 255;
uint16_t fps = 160;
uint8_t speed = 210;
uint8_t density = 10;
uint8_t trail_length = 230;
uint8_t hue_speed = 1;
uint8_t hue = 160;
bool starting = true;
bool idle = true;
long fade_duration = 5000;
uint8_t pattern_duration_min = 5;
uint8_t pattern_duration_max = 15;
uint8_t pattern_duration = random8(pattern_duration_min, pattern_duration_max);
uint8_t glitter_min_on = 1;
uint8_t glitter_max_on = 3;
uint8_t glitter_min_off = 10;
uint8_t glitter_max_off = 30;
long last_run = 0;
bool add_glitter = false;

class Cell {
    public:
        bool alive = 0;
        bool prev = 0;
        uint8_t hue = 0;
        uint8_t saturation = 255;
        uint8_t brightness = 0;
};

Cell world[width][height];

void handle_serial();
void play();
void show();
void render();
void setPixel(uint16_t pixel, byte r, byte g, byte b);

// patterns
void idle_pattern();
void wave_up();
void wave_down();
void wave_left();
void wave_right();
void glitter();
void golPattern();
void randomFillWorld();
int neighbours(int x, int y);
void displayWorld();
void updateWorld();

bool gReverseDirection = false;

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { wave_up, wave_down, wave_left, wave_right };
uint8_t gPatternIndex = random8(NUM_PATTERNS);

CRGB leds[NUM_LEDS];

CRGBPalette16 gPal;

void setup() {
    delay(1000); // sanity delay

    Serial.begin(115200);

#ifdef ESP8266
        esp_leds.Begin();
#else
        octo_leds.begin();
#endif

#ifdef DEBUG
    Serial.println(height);
    Serial.println();
    Serial.println(width);
    Serial.println(NUM_LEDS);
#endif
    Serial.println("OK");
}

void loop() {
    handle_serial();

    EVERY_N_SECONDS(1) {
        Serial.print("[");
        Serial.print(millis());
        Serial.println("]");
        Serial.print("\tidle: ");
        Serial.println(idle);
        // Serial.printf("\tfps: %d\n", fps);
        if (!idle) {
            Serial.print("\tgPatternIndex: ");
            Serial.println(gPatternIndex);
            Serial.print("\tpattern_duration: ");
            Serial.println(pattern_duration);
        }
        if (add_glitter) {
            Serial.print("\tglitter: ");
            Serial.println(add_glitter);
        }
    }

    if (idle) {
        // idle_pattern();
        golPattern();
    }
    else {
        play();
    }

    render();
    show();
    if (!idle) FastLED.delay(1000 / fps);
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
            Serial.print("unkown command: ");
            Serial.println(cmd);
            idle = true;
        }

        // flish the rest if there is any
        Serial.flush();
    }
}

void play() {
    static uint8_t fade_interval = (fade_duration - 1000)/255;
    long now = millis();

    // first fade to black
    if (now - last_run < fade_duration) {
    EVERY_N_MILLISECONDS(fade_interval) {
            fadeToBlackBy(leds, NUM_LEDS, 4);
        }
        return;
    }

    gPatterns[gPatternIndex]();
    // glitter();

    // EVERY_N_SECONDS_I(glitter_timer, random8(glitter_min_on, glitter_max_on)) {
    //         add_glitter = !add_glitter;
    //         if (!add_glitter) {
    //           glitter_timer.setPeriod(random8(glitter_min_off, glitter_max_off));
    //         }
    //         else {
    //           glitter_timer.setPeriod(random8(glitter_min_on, glitter_max_on));
    //         }
    // }

    EVERY_N_SECONDS_I(pattern_timer, pattern_duration) {
        // switch to random pattern
        gPatternIndex = random8(NUM_PATTERNS);

        // choose random length for next pattern
        pattern_duration = random8(pattern_duration_min, pattern_duration_max);
        pattern_timer.setPeriod(pattern_duration);

        Serial.print("switching to pattern index ");
        Serial.print(gPatternIndex);
        Serial.print(" for ");
        Serial.println(pattern_duration);
    }
}

void wave_up() {
    static uint8_t y = 0;
    static uint8_t colour_index = 0;
    static uint8_t base_hue = 0;

    colour_index = base_hue + 2 * y;

#ifdef DEBUG
    Serial.print("y: ");
    Serial.print(y);
    Serial.print("  c: ");
    Serial.println(colour_index);
#endif

    fadeToBlackBy(leds, NUM_LEDS, 255/(height/10));

    for (uint8_t x = 0; x < width; x++) {
        if (x % 2) {
            leds[x * height + y] = ColorFromPalette(RainbowColors_p, colour_index);
        }
        else {
            leds[((x + 1) * height) - 1 - y] = ColorFromPalette(RainbowColors_p, colour_index);
        }
    }

    y = (y + 1) % height;
    base_hue++;
}

void wave_down() {
    static uint8_t y = height - 1;
    static uint8_t colour_index = 0;
    static uint8_t base_hue = 0;

    colour_index = base_hue + 2 * y;

#ifdef DEBUG
    Serial.print("y: ");
    Serial.print(y);
    Serial.print("  c: "
    Serial.println(colour_index);
#endif

    fadeToBlackBy(leds, NUM_LEDS, 255/(height/6));

    for (uint8_t x = 0; x < width; x++) {
        if (x % 2) {
            leds[x * height + y] = ColorFromPalette(RainbowColors_p, colour_index);
        }
        else {
            leds[((x + 1) * height) - 1 - y] = ColorFromPalette(RainbowColors_p, colour_index);
        }
    }

    y = (y - 1) % height;
    base_hue++;
}

void wave_left() {
    static uint8_t x = 0;
    static uint8_t colour_index = 0;
    static uint8_t base_hue = 0;

    colour_index = base_hue + 2 * x;

#ifdef DEBUG
    Serial.print("x: ");
    Serial.print(x);
    Serial.print("  c: ");
    Serial.println(colour_index);
#endif

    // fade out within quarter of width
    fadeToBlackBy(leds, NUM_LEDS, 255/(width/6));

    for (uint8_t y = 0; y < height; y++) {
        leds[x * height + y] = ColorFromPalette(RainbowColors_p, colour_index);
    }

    x = (x + 1) % width;
    base_hue++;
}

void wave_right() {
    static uint8_t x = width - 1;
    static uint8_t colour_index = 0;
    static uint8_t base_hue = 0;

    colour_index = base_hue + 2 * x;

#ifdef DEBUG
    Serial.print("x: ");
    Serial.print(x);
    Serial.print("  c: ");
    Serial.println(colour_index);
#endif

    // fade out within quarter of width
    fadeToBlackBy(leds, NUM_LEDS, 255/(width/6));

    for (uint8_t y = 0; y < height; y++) {
        leds[(width - 1 - x) * height + y] = ColorFromPalette(RainbowColors_p, colour_index);
    }

    x = (x + 1) % width;
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

    fadeToBlackBy(leds, NUM_LEDS, 20);

    EVERY_N_MILLISECONDS(100) {
        for (uint8_t i = 0; i < 50; i++) {
            leds[ random16(NUM_LEDS) ] = ColorFromPalette(gPal, random8());
        }
        // Serial.print("."); //debug
    }
}

void golPattern() {
    static uint32_t last_update = 0;
    uint32_t now = millis();

    // if ((now - last_interaction) > (idle_timeout * 1000)) {
    //   EVERY_N_MILLISECONDS(100) {
    //     brightness++;
    //   }
    //   EVERY_N_MILLISECONDS(125) {
    //     hue_speed++;
    //   }
    //   EVERY_N_MILLISECONDS(200) {
    //     speed++;
    //   }
    //   EVERY_N_MILLISECONDS(111) {
    //     trail_length++;
    //   }
    //   EVERY_N_MILLISECONDS(263) {
    //     saturation++;
    //   }
    //   EVERY_N_MILLISECONDS(155) {
    //     density++;
    //   }
    //   EVERY_N_SECONDS(5) {
    //     update_hue = !update_hue;
    //   }
    // }

    if ((now - last_update) < (255 - speed)) return;
    last_update = now;

    if (starting) {
        uint8_t old_density = density;
        density = 64;
        randomFillWorld();
        density = old_density;
        starting = false;
    }

    displayWorld();
    updateWorld();
}

void displayWorld() {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            // CRGB rgb;
            CHSV hsv(world[x][y].hue, world[x][y].saturation, world[x][y].brightness);
            // hsv2rgb_rainbow(hsv, rgb);
            if (x % 2) {
                // setPixel((x * height + y), rgb.r, rgb.g, rgb.b);
                leds[x * height + y] = hsv;
            }
            else {
                // setPixel(((x + 1) * height - y - 1), rgb.r, rgb.g, rgb.b);
                leds[(x + 1) * height - y - 1] = hsv;
            }
        }
    }
}

void updateWorld() {
    // count total number of alive cells to determine when pattern has died
    uint16_t alive_cells = 0;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {

            // trail off brightness of dead cells
            if (world[x][y].brightness > 0 && world[x][y].prev == 0) {
                if (world[x][y].brightness - (255 - trail_length) < 0) {
                    world[x][y].brightness = 0;
                }
                else {
                    world[x][y].brightness -= (255 - trail_length);
                }
                // world[x][y].hue = hue; // add trigger?
            }

            int count = neighbours(x, y);

            // A new cell is born
            if (count == 3 && world[x][y].prev == 0) {
                world[x][y].alive = 1;
                world[x][y].hue = hue;
                world[x][y].brightness = brightness;
                world[x][y].saturation = saturation;
                alive_cells++;
            }
            // Cell dies
            else if ((count < 2 || count > 3) && world[x][y].prev == 1) {
                world[x][y].alive = 0;
            }
            // just update hue and saturation for alive cells
            // else if (world[x][y].alive && update_hue) {
            //     world[x][y].hue = hue;
            //     alive_cells++;
            // }

            // very low random chance for each cell to be born parentless
            if (random(10000) < (uint16_t)density) {
                world[x][y].alive = 1;
                world[x][y].brightness = brightness;
                world[x][y].hue = hue;
                world[x][y].saturation = 255;
            }
        }
    }

    hue += hue_speed;

    // less than 3 cells cannot spark life
    if (alive_cells < 3) starting = true;

    // std::swap(old_world, world);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            world[x][y].prev = world[x][y].alive;
        }
    }
}

void randomFillWorld() {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (random(255) < (uint16_t)density) {
                world[x][y].alive = 1;
                world[x][y].brightness = brightness;
            }
            else {
                world[x][y].alive = 0;
                world[x][y].brightness = 0;
            }
            world[x][y].prev = world[x][y].alive;
            world[x][y].hue = 0;
        }
    }
}

int neighbours(int x, int y) {
    return
        (world[(x + 1) % width][y].prev) +
        (world[x][(y + 1) % height].prev) +
        (world[(x + width - 1) % width][y].prev) +
        (world[x][(y + height - 1) % height].prev) +
        (world[(x + 1) % width][(y + 1) % height].prev) +
        (world[(x + width - 1) % width][(y + 1) % height].prev) +
        (world[(x + width - 1) % width][(y + height - 1) % height].prev) +
        (world[(x + 1) % width][(y + height - 1) % height].prev);
}

void render() {
    for (uint16_t led = 0; led < NUM_LEDS; led++) {
        setPixel(led, leds[led].r, leds[led].g, leds[led].b);
    }
}

void setPixel(uint16_t pixel, byte r, byte g, byte b) {
#ifdef ESP8266
        esp_leds.SetPixelColor(pixel, RgbColor(r, g, b));
#else
        octo_leds.setPixel(pixel, Color(r, g, b));
#endif
}

void show() {
#ifdef ESP8266
        esp_leds.Show();
#else
        octo_leds.show();
#endif
}
