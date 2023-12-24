#ifndef DISPLAY_H
#define DISPLAY_H

// #include "U8x8lib.h" // schnell, NUR text keine grafik; https://github.com/olikraus/u8g2
// #include <ezButton.h> // https://github.com/ArduinoGetStarted/button

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace MYDISPLAY {

#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// const uint8_t NUM_MENU_ITEMS = 3;
const unsigned long UPDATE_CYCLE = 2000; // [ms]

// extern U8X8_SH1106_128X64_NONAME_HW_I2C oled; // 1,3"
// extern U8X8_SSD1306_128X64_NONAME_HW_I2C oled; // 0,96"
extern Adafruit_SSD1306 display;

// ezButton tasteLinks(32);
// ezButton tasteOben(31); passt vom platz her nicht mehr
// ezButton tasteSelect(31);

void setup();
void loop();


}
#endif