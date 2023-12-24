/**
 * @mainpage
 * TODO
 *
 * Hardware:
 * - ESP32 WROOM mit CP210x USB to UART bridge
 * - RFM95 Lora Modul basierend auf SX1276
 *   MOSI  - D23
 *   MISO  - D19
 *   SCK   - D18
 *   RESET - D14
 *   NSS   - D5
 *   DIO0  - D2
 *
 * Auszug aus dem lorabeispiel:
 * This code uses InvertIQ function to create a simple Gateway/Node logic.
 *
 * Gateway - Sends messages with enableInvertIQ()
 *         - Receives messages with disableInvertIQ()
 *
 * Node    - Sends messages with disableInvertIQ()
 *         - Receives messages with enableInvertIQ()
 *
 * With this arrangement a Gateway never receive messages from another Gateway
 * and a Node never receive message from another Node.
 * Only Gateway to Node and vice versa.
 *
 * This code receives messages and sends a message every second.
 *
 * InvertIQ function basically invert the LoRa I and Q signals.
 *
 * See the Semtech datasheet, http://www.semtech.com/images/datasheet/sx1276.pdf
 * for more on InvertIQ register 0x33.
 */
#include <SPI.h>
#include <LoRa.h> // https://github.com/sandeepmistry/arduino-LoRa

#include "display.h"

const long frequency = 868E6; // LoRa Frequency

const int csPin = 5;   // LoRa radio chip select
const int resetPin = 14; // LoRa radio reset
const int irqPin = 2;   // change for your board; must be a hardware interrupt pin

void LoRa_rxMode()
{
  LoRa.enableInvertIQ(); // active invert I and Q signals
  LoRa.receive();        // set receive mode
}

void LoRa_txMode()
{
  LoRa.idle();            // set standby mode
  LoRa.disableInvertIQ(); // normal mode
}

void LoRa_sendMessage(String message)
{
  LoRa_txMode();        // set tx mode
  LoRa.beginPacket();   // start packet
  LoRa.print(message);  // add payload
  LoRa.endPacket(true); // finish packet and send it
}



void onReceive(int packetSize)
{
  String message = "";

  while (LoRa.available())
  {
    message += (char)LoRa.read();
  }

  Serial.print("Node Receive: ");
  Serial.println(message);
}

void onTxDone()
{
  Serial.println("TxDone");
  LoRa_rxMode();
}

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}


void setup()
{
  Serial.begin(115200); // initialize serial
  while (!Serial)
    ;

  Serial.println(F(">>>>>>>>>>>>>>>>>>>>>>> ESP 32 Lora test V0.1.0 <<<<<<<<<<<<<<<<<<<"));

  Serial.print(F("  Kompilierzeitstempel: "));
  Serial.print(F(__DATE__));
  Serial.print(F(" "));
  Serial.println(F(__TIME__));

  MYDISPLAY::setup();

  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(frequency))
  {
    Serial.println("LoRa init failed. Check your connections.");
    while (true)
      ; // if failed, do nothing
  }

  Serial.println("LoRa init succeeded.");
  Serial.println();
  Serial.println("LoRa Simple Node");
  Serial.println("Only receive messages from gateways");
  Serial.println("Tx: invertIQ disable");
  Serial.println("Rx: invertIQ enable");
  Serial.println();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();

  
}

void loop()
{
  if (runEvery(1000)) { // repeat every 1000 millis
    String message = "HeLoRa World! ";
    message += "I'm a Node! ";
    message += millis();

    LoRa_sendMessage(message); // send a message

    Serial.println("Send Message!");
  }
}

