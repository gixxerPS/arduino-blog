#include "display.h"

using namespace MYDISPLAY;

// U8X8_SH1106_128X64_NONAME_HW_I2C MYDISPLAY::oled(U8X8_PIN_NONE);
// U8X8_SSD1306_128X64_NONAME_HW_I2C MYDISPLAY::oled(U8X8_PIN_NONE);
// TwoWire twi = TwoWire(1); // our own TwoWire instance on bus 1
// Adafruit_SSD1306 MYDISPLAY::display(SCREEN_WIDTH, SCREEN_HEIGHT, &twi, OLED_RST);
Adafruit_SSD1306 MYDISPLAY::display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void MYDISPLAY::setup(void)
{
//   oled.begin();

//   // oled.setFont( u8x8_font_8x13_1x2_f ); // enthaelt "°"; 2376 byte
//   /* enthaelt "°"; 2427 byte
//   * 4 Zeilen moeglich => print idx [y] = [0,2,4,6]
//    * 
//    */
//   oled.setFont( u8x8_font_7x14_1x2_f ); // enthaelt "°"; 2427 byte 4

//   oled.setCursor(0, 0);
//   oled.print(F("Kohlenzug"));

//   oled.setCursor(0, 2);
//   oled.print(F("V0.2.0"));

//   oled.setCursor(0, 4);
//   oled.print(F("----------------"));

//   oled.setCursor(0, 6);
//   oled.print(F("Init"));

//   delay(3000);

//   oled.setCursor(0, 0);
//   oled.print(F("Automatik: AUS"));

//   oled.setCursor(0, 2);
//   oled.print(F("Schritt  :"));

//   oled.setCursor(0, 6);
//   oled.print(F("Warte auf Freig"));


// pinMode(OLED_RST, OUTPUT);
//   digitalWrite(OLED_RST, LOW);
//   delay(20);
//   digitalWrite(OLED_RST, HIGH);
Serial.println(F("SSD1306 init"));
    // twi.begin(OLED_SDA, OLED_SCL);  // Needs to come before display.begin is used
    Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA SENDER ");
  display.display();

}

void MYDISPLAY::loop(void)
{
    
}
