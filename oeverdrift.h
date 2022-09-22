// Copyright (c) 2022 Jan Lindblom <jan@namnlos.co>
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#define LED_PIN 0
#define NUM_LEDS 16
#define LED_ADDR 0

#define KEY_PGDN 3
#define KEY_GUI 4
#define KEY_PGUP 5

#define LIGHT_LEVELS 6
#define LIGHT_ADDR LED_ADDR + (NUM_LEDS * 2)

void set_brightness(uint8_t brightness);
void decrease_brightness();
void increase_brightness();
void key_actions();
void keyboard_handler();

typedef union {
  uint32_t raw;
  struct {
    char char1 : 8;
    char char2 : 8;
    char char3 : 8;
    char char4 : 8;
  };
} packchars;
