/**
 * @mainpage
 * TODO
 * 
 * Hardware:
 * - ESP32 WROOM mit CP210x USB to UART bridge
 * 
 * App zum Auslesen zb:
 * https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=de&gl=US&pli=1
 */
#include <Arduino.h>
#include "BluetoothSerial.h"

const String BLUETOOTH_NAME = "ESP32test";

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println(F(">>>>>>>>>>>>>>>>>>>>>>> ESP 32 Bluetooth test V0.1.0 <<<<<<<<<<<<<<<<<<<"));

  Serial.print(F("  Kompilierzeitstempel: "));
  Serial.print(F(__DATE__));
  Serial.print(F(" "));
  Serial.println(F(__TIME__));

  SerialBT.begin(BLUETOOTH_NAME); 
  Serial.println("Der ESP32 ist bereit. Verbinde dich nun über Bluetooth.");
}

void loop() {
  static uint32_t counter = 0;
  SerialBT.print("Lebenszeichenzaehler: ");
  SerialBT.println(counter++);

  delay(1000);
}
