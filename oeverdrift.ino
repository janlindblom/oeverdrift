// Copyright (c) 2022 Jan Lindblom <jan@namnlos.co>
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "FIFO.h"
#include "oeverdrift.h"
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

uint32_t warmer_white = 0xFF7847;
uint32_t warm_white = 0xFF785A;
uint32_t white = 0xFFFFFF;

enum modes { WHITE, WARM, HOT };

enum mode_style { STATIC, DYNAMIC };

uint8_t mode = WHITE;

uint32_t pattern_white[] = {0xFF7847, 0xFF7847, 0xFF7847, 0xFF7847,
                            0xFF7847, 0xFF7847, 0xFF7847, 0xFF7847,
                            0xFF7847, 0xFF7847, 0xFF7847, 0xFF7847,
                            0xFF7847, 0xFF7847, 0xFF7847, 0xFF7847};
uint32_t pattern_warmwhite[] = {0xFF6433, 0xFF6433, 0xFF6433, 0xFF6433,
                                0xFF6433, 0xFF6433, 0xFF6433, 0xFF6433,
                                0xFF6433, 0xFF6433, 0xFF6433, 0xFF6433,
                                0xFF6433, 0xFF6433, 0xFF6433, 0xFF6433};
uint32_t pattern_hotwhite[] = {0xFF501F, 0xFF501F, 0xFF501F, 0xFF501F,
                               0xFF501F, 0xFF501F, 0xFF501F, 0xFF501F,
                               0xFF501F, 0xFF501F, 0xFF501F, 0xFF501F,
                               0xFF501F, 0xFF501F, 0xFF501F, 0xFF501F};

// uint32_t (*)[] modes[] = {, &pattern_warmwhite, &pattern_hotwhite};
uint8_t mode_styles[] = {STATIC, STATIC, STATIC};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB);

uint8_t sine[] = {4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5};
uint32_t led_state[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint16_t hue = 0;
uint8_t light_level[] = {0x00, 0x33, 0x66, 0x99, 0xCC, 0xFF};

uint8_t brightness_level = 1;
uint8_t col = 0;

// Our keymap
uint8_t key_map[] = {KEY_LEVEL, KEY_ONOFF, KEY_MODE};
// Initial states of keys (HIGH since inputs are pulled up)
uint8_t key_state_previous[] = {HIGH, HIGH, HIGH};
FIFO key_buf;

void paint(uint32_t pattern[]) {
  for (uint8_t i = 0; i < sizeof(led_state) / sizeof(led_state[0]); i++) {
    strip.setPixelColor(i, pattern[i]);
    led_state[i] = pattern[i];
  }

  strip.show();
}

void setup() {
  // Setup the ÖVERDRIFT
  EEPROM.begin(512);

  for (uint8_t key = 0; key < sizeof(key_map) / sizeof(key_map[0]); key++) {
    pinMode(key_map[key], PinMode::INPUT_PULLUP);
  }
  delay(100);

  strip.begin();
  // for (uint8_t i = 0; i < NUM_LEDS; i++) {

  // strip.setPixelColor(i, wew);
  //    EEPROM.write(LED_ADDR + i, hue >> 8);
  //    EEPROM.write(LED_ADDR + i * 2, hue & 0xFF);
  // }
  paint(pattern_hotwhite);

  brightness_level = EEPROM.read(LIGHT_ADDR);
  strip.setBrightness(light_level[brightness_level]);

  // Keyboard.begin();
  if (!Serial) {
    Serial.begin(9600);
  }
}

void loop() {
  keyboard_handler();
  key_actions();
}

void key_actions() {
  while (key_buf.size() > 0) {
    uint8_t key = key_buf.pop();
    if (key == KEY_LEVEL) {
      increase_brightness();
    }
    if (key == KEY_ONOFF) {
      if (brightness_level != 0) {
        set_brightness(0);
      } else {
        set_brightness(brightness_level);
      }
    }
    if (key == KEY_MODE) {
    }
  }
}

/**
   Keyboard scanner routine.
*/
void keyboard_handler() {
  for (uint8_t key = 0; key < sizeof(key_map) / sizeof(key_map[0]); key++) {
    uint8_t state = digitalRead(key_map[key]);

    if ((state != key_state_previous[key]) && (state != HIGH)) {
      key_buf.push(key_map[key]);
      if (Serial) {
        Serial.print("Key ");
        Serial.print(key_map[key], DEC);
        Serial.print(" state: ");
        Serial.println(state, DEC);
        Serial.println("Registering keypress.");
      }
    }
    key_state_previous[key] = state;
  }
}

void set_brightness(uint8_t brightness) {
  EEPROM.write(LIGHT_ADDR, brightness);
  strip.setBrightness(light_level[brightness]);
  EEPROM.commit();
}

void decrease_brightness() {
  if (brightness_level < 1) {
    brightness_level = 0;
  } else {
    brightness_level--;
  }
  set_brightness(brightness_level);
}

void increase_brightness() {
  if (brightness_level > 3) {
    brightness_level = 1;
  } else {
    brightness_level++;
  }
  set_brightness(brightness_level);
}
