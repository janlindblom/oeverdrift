// Copyright (c) 2022 Jan Lindblom <jan@namnlos.io>
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include "FIFO.h"
#include "oeverdriven.h"

HSVColour white;
HSVColour warmWhite;
HSVColour warmerWhite;
rgb_colour rgb_warmwhite;

uint32_t pattern_white[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint32_t pattern_warmwhite[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint32_t pattern_hotwhite[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB);

uint8_t sine[] PROGMEM = {4, 3, 2, 1, 0, 11, 10, 9, 8, 7, 6, 5};
uint32_t led_state[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint16_t hue = 0;
uint8_t light_level[] = {0x00, 0x33, 0x66, 0x99, 0xCC, 0xFF};

uint8_t brightness_level = 1;

uint32_t wew = 0xFF7847;
uint32_t ww = 0xFF785A;
uint32_t w = 0xFFFFFF;
uint32_t ww_rgb = 0xFDF4DC;

// Our keymap
uint8_t key_map[] = {KEY_PGDN, KEY_GUI, KEY_PGDN};
// Initial states of keys (HIGH since inputs are pulled up)
uint8_t key_state_previous[] = {HIGH, HIGH, HIGH};
FIFO key_buf;

void paint(uint32_t pattern[]) {
  if (sizeof(pattern) > sizeof(led_state)) {
    return;
  }

  Serial.print("Pattern: [");
  for (uint8_t i = 0; i < sizeof(led_state) / sizeof(led_state[0]); i++) {
    strip.setPixelColor(i, pattern[i]);
    led_state[i] = pattern[i];
    Serial.print(pattern[i], HEX);
    if (i + 1 < sizeof(led_state) / sizeof(led_state[0])) {
      Serial.print(", ");
    }
  }
  Serial.println("]");

  strip.show();
}

void key_actions();
void keyboard_handler();

void setup() {
  // Setup the ÖVERDRIVEN
  //setup_colours();
  warmerWhite.raw = wew;
  warmWhite.raw = ww;
  white.raw = w;
  rgb_warmwhite.raw = ww_rgb;
  EEPROM.begin(512);

  for (uint8_t key = 0; key < sizeof(key_map) / sizeof(key_map[0]); key++) {
    pinMode(key_map[key], INPUT_PULLUP);
  }
  delay(200);

  Serial.begin(9600);
  while (!Serial);

  strip.begin();
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    pattern_hotwhite[i] = wew;
    //strip.setPixelColor(i, wew);
    //   EEPROM.write(LED_ADDR + i, hue >> 8);
    //   EEPROM.write(LED_ADDR + i * 2, hue & 0xFF);
  }
  paint(pattern_hotwhite);

  // brightness_level = EEPROM.read(LIGHT_ADDR);
  strip.setBrightness(light_level[brightness_level]);

  //Keyboard.begin();

  Serial.print("Warmer White: 0x");
  Serial.println(wew, HEX);
  Serial.print("Warm White: 0x");
  Serial.println(ww, HEX);
  Serial.print("White: 0x");
  Serial.println(w, HEX);
  Serial.print("Warm White (RGB): 0x");
  Serial.println(ww_rgb, HEX);
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

packchars* pack_message(String str, packchars entries[]) {
  unsigned int len = str.length();
  char str_chars[len];
  str.toCharArray(str_chars, len);
  //packchars entries[(unsigned int)(len / 4 + (len % 4))];

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
  if (key_buf.size() > 0) {
    uint8_t key = key_buf.pop();
    if (key == KEY_PGDN) {
      Serial.println("PGDN");
    }
    if (key == KEY_GUI) {
      Serial.println("GUI");
    }
    if (key == KEY_PGUP) {
      Serial.println("PGUP");
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
      /*
        String msg = String("Key pressed: 0x" + String(key_map[key], HEX));
        int packed_length = (int)(msg.length() / 4 + (msg.length() % 4));
        packchars * packed_msg = (packchars*) malloc(packed_length);
        for (int i = 0; i < packed_length; i++) {
        rp2040.fifo.push_nb(packed_msg[i].raw);
        }
      */
      Serial.print("Key pressed: 0x");
      Serial.println(key_map[key], HEX);

      key_buf.push(key_map[key]);
    }
    key_state_previous[key] = state;
  }
}

void setup_colours() {
  white.raw = 0;
  warmWhite.raw = 0;
  warmerWhite.raw = 0;

  white.hue = 0;
  warmWhite.hue = 2002;
  warmerWhite.hue = 2912;

  white.sat = 0;
  warmWhite.sat = 165;
  warmerWhite.sat = 184;

  white.val = 255;
  warmWhite.val = 255;
  warmerWhite.val = 255;
}

int sort_desc(const void *cmp1, const void *cmp2) {
  double_t a = *((double *)cmp1);
  double_t b = *((double *)cmp2);
  // The comparison
  return a > b ? -1 : (a < b ? 1 : 0);
}

HSVColour create_hsv_colour(uint16_t h, uint8_t s, uint8_t v) {
  HSVColour shade;
  shade.raw = 0;
  shade.hue = h;
  shade.sat = s;
  shade.val = v;
  return shade;
}

HSVColour rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b) {
  double_t rd = ((double_t)r) / 255;
  double_t gd = ((double_t)g) / 255;
  double_t bd = ((double_t)b) / 255;
  double_t rgbd[] = {rd, gd, bd};
  qsort(rgbd, sizeof(rgbd) / sizeof(rgbd[0]), sizeof(rgbd[0]), sort_desc);
  double_t c_max = rgbd[0];
  double_t c_min = rgbd[2];
  double_t delta = c_max - c_min;
  uint8_t saturation = c_max != 0 ? (uint8_t)(255 * (delta / c_max)) : 0;
  uint8_t value = (uint8_t)(c_max * 255);
  double_t h = 0;

  if (rd == c_max) {
    h = (gd - bd) / delta;
    h = (int16_t)h % 6;
  } else if (gd == c_max) {
    h = (bd - rd) / delta;
    h += 2;
  } else if (bd == c_max) {
    h = (rd - gd) / delta;
    h += 4;
  }
  h *= 60;
  if (h < 0) {
    h += 360;
  }
  h = (65536 / 360) * h;

  return create_hsv_colour((uint16_t)h, saturation, value);
}

uint32_t colour(Adafruit_NeoPixel np, HSVColour hsv) {
  return np.ColorHSV(hsv.hue, hsv.sat, hsv.val);
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
