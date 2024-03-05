#include <EEPROM.h>
#include <FastLED.h>
#include <PinButton.h>

#define DATA_PIN 6       // Define the pin to which the data line of WS2812 LEDs is connected
#define LED_COUNT 18     // Number of WS2812 LEDs (change this to match your actual number of LEDs)
#define BUTTON_PIN 2     // Pin to which the button is connected

#define LOOP_TIME 20     // Time in ms delay between each logic loop

#define MAX_BRIGHTNESS 255
#define MIN_BRIGHTNESS 10
#define FADE_AMOUNT    2    // Amount to fade up and down each tick

#define RAINBOW_SPEED 20

// Saving to flash //
// Version string. Increment this to allow new formats of config to be written to the EEPROM
#define CONFIG_VERSION "VER01"
#define CONFIG_START 32         // Location of config in EEPROM
#define CONFIG_DEFAULT_BRI 128  // Default brightness to be used if flash can't be read
#define CONFIG_DEFAULT_COLOR 1  // Default color state to be used if flash can't be read

typedef struct {
  char version[6]; // Allows to detect if config is actually written and is not garbage or outdated version
  byte currentColor;
  int brightness;
} config_type;

// Default config values
config_type CONFIGURATION = {
  CONFIG_VERSION,
  CONFIG_DEFAULT_COLOR,
  CONFIG_DEFAULT_BRI
};


PinButton myButton(BUTTON_PIN);

CRGB leds[LED_COUNT];    // Define the LED array
uint8_t rainbowHue = 0;  // Initial hue value for the rainbow effect

bool inLongClick = false;
bool brightnessIncreasing = true;


int loadConfig() {
  // Load what's in EEPROM in to the local CONFIGURATION if it is a valid setting
  // Return 0 if config loaded, otherwise return 1

  // Check for version string
  for (int i=0; i<sizeof(CONFIGURATION.version); i++){
    if (EEPROM.read(CONFIG_START + i) != CONFIG_VERSION[i])
      return 1;
  }

  // Load into (overwrite) the local configuration struct
  EEPROM.get(CONFIG_START, CONFIGURATION);
  return 0;
}

void saveConfig() {
  // Save the CONFIGURATION to EEPROM
  EEPROM.put(CONFIG_START, CONFIGURATION);
}


void rainbow() {
  fill_rainbow(leds, LED_COUNT, rainbowHue, RAINBOW_SPEED);
  rainbowHue++;
}


void setup() {
  // Attempt to load the config
  if(!loadConfig()){
    // Config loaded
  } else {
    // Config not loaded. Attempt to overwrite flash with the default settings
    saveConfig();
  }

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, LED_COUNT); // Initialize FastLED
  FastLED.setBrightness(CONFIGURATION.brightness);
  FastLED.show();                                          // Initialize LEDs
}

void loop() {
  myButton.update();  // Call this at least every ~20ms

  if (myButton.isSingleClick()) {
    CONFIGURATION.currentColor = (CONFIGURATION.currentColor + 1) % 10;  // Cycle through colors
    saveConfig();  // Save the new color to flash
  }
  if (myButton.isDoubleClick()) {
    CONFIGURATION.currentColor = 0;  // Reset color to Black (turn off)
  }
  if (myButton.isLongClick()) {
    inLongClick = true;
  }
  if (myButton.isReleased()) {
    if (inLongClick) {
      inLongClick = false;
      brightnessIncreasing = true;
      saveConfig();  // Save the new brightness to flash
    }
  }


  if (inLongClick) {
    if (brightnessIncreasing) {
      CONFIGURATION.brightness += FADE_AMOUNT;
      if (CONFIGURATION.brightness > MAX_BRIGHTNESS) {
        CONFIGURATION.brightness = MAX_BRIGHTNESS;
        brightnessIncreasing = false;
      }
    } else {
      CONFIGURATION.brightness -= FADE_AMOUNT;
      if (CONFIGURATION.brightness < MIN_BRIGHTNESS) {
        CONFIGURATION.brightness = MIN_BRIGHTNESS;
        brightnessIncreasing = true;
      }
    }
  }

  switch (CONFIGURATION.currentColor) {
    case 0:
      fill_solid(leds, LED_COUNT, CRGB::Black); // Set LEDs to black
      break;
    case 1:
      fill_solid(leds, LED_COUNT, CRGB::Red);
      break;
    case 2:
      fill_solid(leds, LED_COUNT, CRGB::Green);
      break;
    case 3:
      fill_solid(leds, LED_COUNT, CRGB::Blue);
      break;
    case 4:
      fill_solid(leds, LED_COUNT, CRGB::Cyan);
      break;
    case 5:
      fill_solid(leds, LED_COUNT, CRGB::Magenta);
      break;
    case 6:
      fill_solid(leds, LED_COUNT, CRGB::Yellow);
      break;
    case 7:
      fill_solid(leds, LED_COUNT, CRGB(255, 60, 0)); // Orange: Red 255, Green 60, Blue 0
      break;
    case 8:
      fill_solid(leds, LED_COUNT, CRGB(200, 60, 200)); // Pink: Red 200, Green 60, Blue 200
      break;
    case 9:
      rainbow();
      break;
  }

  FastLED.setBrightness(CONFIGURATION.brightness);
  FastLED.show();
  delay(LOOP_TIME);
}
