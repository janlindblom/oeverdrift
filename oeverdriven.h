// Copyright (c) 2022 Jan Lindblom <jan@namnlos.io>
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

void setup_colours();
void set_brightness(uint8_t brightness);
void decrease_brightness();
void increase_brightness();

typedef union {
  uint32_t raw;
  struct {
    uint16_t hue : 16;
    uint8_t sat : 8;
    uint8_t val : 8;
  };
} HSVColour;

typedef union {
  uint32_t raw;
  struct {
    uint8_t r : 8;
    uint8_t g : 8;
    uint8_t b : 8;
    uint8_t w : 8;
  };
} rgb_colour;

typedef union {
  uint32_t raw;
  struct {
    char char1: 8;
    char char2: 8;
    char char3: 8;
    char char4: 8;
  };
} packchars;

#define LED_PIN 0
#define NUM_LEDS 12
#define LED_ADDR 0

#define KEY_PGDN 3
#define KEY_GUI 4
#define KEY_PGUP 5

#define LIGHT_LEVELS 6
#define LIGHT_ADDR 24
