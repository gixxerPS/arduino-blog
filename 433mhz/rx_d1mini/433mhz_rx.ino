/*
 * Hardware:
 * - D1 mini
 * - RXB8 433MHz empfaenger
 *     D2 Data
 */

// #include <RH_ASK.h> // https://github.com/PaulStoffregen/RadioHead
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
    Serial.begin(74880); // optional: aber praktisch fuer debug ausgaben ;)
    while (!Serial) ; 
    delay(200);

    Serial.print(">>>>>>>>>>>>>>>>>>>> (Neu-)Start ...");
    pinMode(LED_BUILTIN, OUTPUT);

    // if (!driver.init()) {
    //     Serial.println("init failed");
    // }

    mySwitch.enableReceive(4);  // Receiver on interrupt 4 => that is pin D2 on D1 mini
}

void loop()
{
    // uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    // uint8_t buflen = sizeof(buf);
    // static int i;
    // if (driver.recv(buf, &buflen)) // Non-blocking
    // {
    //     i++;
    //     Serial.print(i);
    //     // Message with a good checksum received, dump it.
    //     driver.printBuffer(" Got:", buf, buflen);
    // }

    if (mySwitch.available()) {
        Serial.print("Received ");
        Serial.print( mySwitch.getReceivedValue() );
        Serial.print(" / ");
        Serial.print( mySwitch.getReceivedBitlength() );
        Serial.print("bit ");
        Serial.print("Protocol: ");
        Serial.println( mySwitch.getReceivedProtocol() );
        mySwitch.resetAvailable();
    }
}