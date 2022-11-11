// Copyright (c) 2022 Jan Lindblom <jan@namnlos.co>
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <AceButton.h>
using namespace ace_button;

#define LED_PIN 0
#define NUM_LEDS 16

#define KEY_LEVEL 3
#define KEY_ONOFF 4
#define KEY_MODE 5

#define LIGHT_LEVELS 6
#define MODES 3
#define LIGHT_ADDR 0
#define MODE_ADDR LIGHT_ADDR + 1
#define LED_ADDR MODE_ADDR + 1

void set_brightness(uint8_t brightness);
void increase_brightness();
void key_actions();
void keyboard_handler();
void handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState);
void rotate_mode();
void set_mode();