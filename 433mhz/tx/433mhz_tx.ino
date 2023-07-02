/*
 * Hardware:
 * - arduino mega PRO mini
 * - 433MHz sender
 *     D8
 */

// #include <RH_ASK.h>
// #ifdef RH_HAVE_HARDWARE_SPI
//     #include <SPI.h>
// #endif

// RH_ASK driver(/*uint16_t speed*/2000, 
//             /*uint8_t rxPin*/9, 
//             /*uint8_t txPin*/8, 
//             /*uint8_t pttPin*/10);

#include <RCSwitch.h> // https://github.com/sui77/rc-switch/
RCSwitch mySwitch = RCSwitch();

void setup()
{
    Serial.begin(9600); // optional: aber praktisch fuer debug ausgaben ;)
    while (!Serial) ; 
    delay(200);
   
    Serial.print(">>>>>>>>>>>>>>>>>>>> (Neu-)Start ...");
    pinMode(LED_BUILTIN, OUTPUT);

    // if (!driver.init()) {
    //     Serial.println("init failed");
    // }

    mySwitch.enableTransmit(8);
}

void loop()
{
    static int ledState = HIGH;

    // const char *msg = "hello";

    // driver.send((uint8_t *)msg, strlen(msg));
    // driver.waitPacketSent();

    mySwitch.switchOff("11111", "00010");

    digitalWrite(LED_BUILTIN, ledState);
    ledState = !ledState;
    delay(500);
}