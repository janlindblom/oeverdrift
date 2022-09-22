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
uint32_t colors[] = {white, warm_white, warmer_white};

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

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB);

uint8_t sine[] = {4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5};
uint32_t led_state[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint16_t hue = 0;
uint8_t light_level[] = {0x00, 0x33, 0x66, 0x99, 0xCC, 0xFF};

uint8_t brightness_level = 1;
uint8_t col = 0;

// Our keymap
uint8_t key_map[] = {KEY_PGDN, KEY_GUI, KEY_PGUP};
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
    pinMode(key_map[key], INPUT_PULLUP);
  }
  delay(100);

  strip.begin();
  // for (uint8_t i = 0; i < NUM_LEDS; i++) {

  // strip.setPixelColor(i, wew);
  //    EEPROM.write(LED_ADDR + i, hue >> 8);
  //    EEPROM.write(LED_ADDR + i * 2, hue & 0xFF);
  // }
  paint(pattern_hotwhite);

  // brightness_level = EEPROM.read(LIGHT_ADDR);
  strip.setBrightness(light_level[brightness_level]);

  // Keyboard.begin();
}

void loop() {
  // uint32_t color = strip.ColorHSV(hue, 33, 252);
  // for (uint8_t i = 0; i < NUM_LEDS; i++) {
  // if (hue >= 65407) {
  //   hue = 0;
  // }
  //  delay(100);
  //  strip.setPixelColor(sine[i], color);
  //   EEPROM.update(LED_ADDR + i, hue >> 8);
  //   EEPROM.update(LED_ADDR + i * 2, hue & 0xFF);
  //  hue += 128;
  //}

  // strip.fill(color);
  // strip.show();
  // EEPROM.commit();

  keyboard_handler();
  key_actions();
  /*
    if (digitalRead(KEY_PGDN) != HIGH) {
    Serial.println("PGDN");
    decrease_brightness();
    }

    if (digitalRead(KEY_GUI) != HIGH) {
    Serial.println("GUI");
    if (strip.getBrightness() != 0) {
      set_brightness(0);
    } else {
      set_brightness(brightness_level);
    }
    }

    if (digitalRead(KEY_PGUP) != HIGH) {
    Serial.println("PGUP");
    increase_brightness();
    }
  */
}

void loop1() {
  delay(1000);
  if (col == 0) {
    paint(pattern_white);
  } else if (col == 1) {
    paint(pattern_warmwhite);
  } else if (col == 2) {
    paint(pattern_hotwhite);
  }
  col++;
  if (col > 2) {
    col = 0;
  }
}

packchars pack_string(String str) {
  packchars entry;
  entry.raw = 0;
  int len = str.length();
  char str_chars[len];
  str.toCharArray(str_chars, len);
  entry.char1 = str_chars[0];
  if (len > 1) {
    entry.char2 = str_chars[1];
    if (len > 2) {
      entry.char3 = str_chars[2];
      if (len > 3) {
        entry.char4 = str_chars[3];
      }
    }
  }
  return entry;
}

packchars *pack_message(String str, packchars entries[]) {
  unsigned int len = str.length();
  char str_chars[len];
  str.toCharArray(str_chars, len);
  // packchars entries[(unsigned int)(len / 4 + (len % 4))];

  uint8_t chars = 4;
  unsigned int packed_index = 0;
  char char_word[4];

  for (int i = 0; i < len; i++) {
    if ((chars < 1) || (i == len - 1)) {
      chars = 4;
      packchars entry = pack_string(char_word);
      entries[packed_index] = entry;
      for (uint8_t c = 0; c < 4; c++) {
        char_word[c] = 0;
      }
      packed_index++;
    }
    char_word[4 - chars] = str_chars[i];
    chars--;
  }
  return entries;
}

void key_actions() {
  while (key_buf.size() > 0) {
    uint8_t key = key_buf.pop();
    if (key == KEY_PGDN) {
      decrease_brightness();
    }
    if (key == KEY_GUI) {
      // Toggle on/off
    }
    if (key == KEY_PGUP) {
      increase_brightness();
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
    }
    key_state_previous[key] = state;
  }
}

void set_brightness(uint8_t brightness) {
  // EEPROM.write(LIGHT_ADDR, brightness);
  strip.setBrightness(light_level[brightness]);
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
    brightness_level = 4;
  } else {
    brightness_level++;
  }
  set_brightness(brightness_level);
}
