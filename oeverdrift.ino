// Copyright (c) 2022 Jan Lindblom <jan@namnlos.co>
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "oeverdrift.h"
#include <AceButton.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

using namespace ace_button;

uint32_t warmer_white = 0xFF501F;
uint32_t warm_white = 0xFF6433;
uint32_t white = 0xFF7847;
uint32_t black = 0;

enum modes { OFF,
             WHITE,
             WARM,
             WARMER };

enum mode_style { STATIC,
                  DYNAMIC };

uint32_t pattern_off[NUM_LEDS] = { black, black, black, black,
                                   black, black, black, black,
                                   black, black, black, black,
                                   black, black, black, black };
uint32_t pattern_white[NUM_LEDS] = { white, white, white, white,
                                     white, white, white, white,
                                     white, white, white, white,
                                     white, white, white, white };
uint32_t pattern_warmwhite[NUM_LEDS] = { warm_white, warm_white, warm_white, warm_white,
                                         warm_white, warm_white, warm_white, warm_white,
                                         warm_white, warm_white, warm_white, warm_white,
                                         warm_white, warm_white, warm_white, warm_white };
uint32_t pattern_warmerwhite[NUM_LEDS] = { warmer_white, warmer_white, warmer_white, warmer_white,
                                           warmer_white, warmer_white, warmer_white, warmer_white,
                                           warmer_white, warmer_white, warmer_white, warmer_white,
                                           warmer_white, warmer_white, warmer_white, warmer_white };

// uint32_t (*)[] modes[] = {, &pattern_warmwhite, &pattern_hotwhite};
uint8_t mode_styles[] = { STATIC, STATIC, STATIC };

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB);

uint8_t sine[NUM_LEDS] = { 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5 };
uint32_t led_state[NUM_LEDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

uint8_t light_level[6] = { 0x00, 0x33, 0x66, 0x99, 0xCC, 0xFF };

volatile uint8_t brightness_level = 3;
volatile uint8_t current_mode = WARM;
volatile bool lamp_is_on = true;

AceButton level_button;
AceButton onoff_button;
AceButton mode_button;

void paint(uint32_t pattern[]) {
  for (uint8_t i = 0; i < sizeof(led_state) / sizeof(led_state[0]); i++) {
    strip.setPixelColor(i, pattern[i]);
    led_state[i] = pattern[i];
  }
  strip.show();
}

void setup() {
  // Setup the ÖVERDRIFT
  // Start a Serial
  if (!Serial) {
    Serial.begin(9600);
  }
  delay(100);

  EEPROM.begin(2 + (NUM_LEDS * 2));

  pinMode(KEY_LEVEL, INPUT_PULLUP);
  pinMode(KEY_ONOFF, INPUT_PULLUP);
  pinMode(KEY_MODE, INPUT_PULLUP);
  level_button.init(KEY_LEVEL);
  onoff_button.init(KEY_ONOFF);
  mode_button.init(KEY_MODE);

  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);

  // Start the light strip
  strip.begin();

  current_mode = EEPROM.read(MODE_ADDR);
  if (current_mode == OFF) {
    current_mode = WARM;
  }
  if (Serial) {
    Serial.print("Mode: ");
    Serial.println(current_mode);
  }
  set_mode();

  // Set brightness level
  brightness_level = EEPROM.read(LIGHT_ADDR);
  if (Serial) {
    Serial.print("Brightness: ");
    Serial.println(brightness_level);
  }
  strip.setBrightness(light_level[brightness_level]);
  strip.show();
}

void loop() {
  // Button scanner loop
  onoff_button.check();
  level_button.check();
  mode_button.check();
}

void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (!Serial) {
    Serial.begin(9600);
  }
  switch (eventType) {
    case AceButton::kEventClicked:
      if (Serial) {
        Serial.print("Button: ");
        Serial.print(button->getPin());
        Serial.print(", event: ");
        Serial.print(eventType);
        Serial.print(", buttonState: ");
        Serial.println(buttonState);
      }
      if (button->getPin() == KEY_LEVEL) {
        increase_brightness();
      } else if (button->getPin() == KEY_ONOFF) {
        if (Serial) {
          Serial.printf("Lamp on? %b\n", lamp_is_on);
        }
        if (lamp_is_on) {
          paint(pattern_off);
          set_brightness(0);
          strip.show();
        } else {
          set_mode();
          set_brightness(brightness_level);
          strip.show();
        }
        lamp_is_on = !lamp_is_on;
      } else if (button->getPin() == KEY_MODE) {
        rotate_mode();
      }
      break;
    case AceButton::kEventDoubleClicked:
      if (Serial) {
        Serial.printf("Double click: %d, buttonState: %d\n", button->getPin(), buttonState);
      }
  }
}

void set_brightness(uint8_t brightness) {
  if (Serial) {
    Serial.print("Setting brightness: ");
    Serial.println(brightness);
  }
  strip.setBrightness(light_level[brightness]);
  strip.show();
  if (brightness > 0) {
    EEPROM.write(LIGHT_ADDR, brightness);
    EEPROM.commit();
  }
}

void increase_brightness() {
  if (!lamp_is_on) {
    return;
  }
  if (brightness_level > 4) {
    brightness_level = 1;
  } else {
    brightness_level++;
  }
  set_brightness(brightness_level);
}

void set_mode() {
  if (!lamp_is_on) {
    return;
  }
  if (Serial) {
    Serial.print("Setting mode: ");
    Serial.println(current_mode);
  }
  switch (current_mode) {
    case WHITE:
      paint(pattern_white);
      break;
    case WARM:
      paint(pattern_warmwhite);
      break;
    case WARMER:
      paint(pattern_warmerwhite);
      break;
  }
}

void rotate_mode() {
  if (!lamp_is_on) {
    return;
  }
  current_mode++;
  if (current_mode >= WARMER) {
    current_mode = WHITE;
  }
  set_mode();
  EEPROM.write(MODE_ADDR, current_mode);
  EEPROM.commit();
}