/*
 * Hardware:
 * - D1 mini (ESP8266)
 * - Data logger shield
 *   * RTC (DS1307)
 *     D1 SCL
 *     D2  SDA
 *   * microSD
 *     D5 CLK
 *     D6  MISO
 *     D7  MOSI
 *     D8  CS
 *   * Temperatursensor BME280
 *     D1 SCL
 *     D2 SCA
 *     6 polige BME Variante versorgt mit 3,3V
 *     CSB = 3,3V (pullup)
 *     SDO = 3,3V (pullup)
 *
 */
#include <Wire.h> // fuer I2C
#include <Adafruit_BME280.h>

// lcd display include
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

Adafruit_BME280 bme1;
float temp1(NAN), hum1(NAN), press1(NAN);

bool bme1SensorError = false; // bme sensor nicht vorhanden oder nicht ok?

// lcd defines
#define OLED_RESET -1
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// #define D8 8 /// ACHTUNG ZEILE LOESCHEN WENN BOARD WIEDER D1 MINI!!!!
#define CS D8
#define countof(a) (sizeof(a) / sizeof(a[0]))

void setup()
{

    Serial.begin(9600);
    // while (!Serial) ; // wait for serial
    delay(1000);
    Serial.print("compiled: "); // kompilier zeitstempel ausgeben
    Serial.print(__DATE__);
    Serial.println(__TIME__);
    pinMode(CS, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    int i;
    for (i = 0; i < 5; i++) { // max 5x versuchen mit bme zu kommunizieren / initialisieren
        if (!bme1.begin()) { // KEIN erfolg?
            Serial.println("BME280 sensor nicht gefunden. Versuche es in 1s noch einmal.");
            delay(1000); // dann 1s warten
        } else {
            break; // nicht weiter probieren
        }
    }
    if (i >= 5) { // 5x ohne erfolg versucht zu kommunizieren?
        bme1SensorError = true;
        Serial.println("Konnte BME280 Sensor nicht finden!");
    } else {
        Serial.println("BME280 Sensor gefunden! ERFOLG!!");
    }

    // display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    // display.setTextSize(2);
    // display.setTextColor(WHITE);
    // display.clearDisplay();
    // display.setCursor(5, 0);
    // display.println("Hallo");
    // display.setTextSize(2);
    // display.setCursor(5, 30);
    // display.print("Datenlogger");
    // display.display(); // Text zeigen
    delay(1000);
}
void print2digits(int number)
{
    if (number >= 0 && number < 10) {
        Serial.write('0');
    }
    Serial.print(number);
}

void loop()
{
    // displayTime(now);
    Serial.println();

    readAndPrintBME280Data();

    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(2000);                     // wait for a second
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
    delay(2000);
}

void readAndPrintBME280Data()
{
    // globale variablen aktualisieren
    temp1 = bme1.readTemperature();
    hum1 = bme1.readHumidity();
    press1 = bme1.readPressure() / 100;

    String Temperatur = String(temp1);
    Serial.print("Temp = " + Temperatur + " °C | ");

    String feuchte = String(hum1);
    Serial.print("Feuchte = " + feuchte + " %RF | ");

    // ohne nachkommastellen darstellen und dabei richtig runden
    String druck = String(int(press1 + 0.5f));
    Serial.println("Druck = " + druck + " hPa");

    // lcd
    // display.clearDisplay();
    // display.setTextSize(2);
    // display.setTextColor(WHITE);
    // display.setCursor(1, 1);
    // display.println(Temperatur + " C");
    // // display.display(); // auftrag ausfuehren
    // display.setCursor(1, 25);
    // display.println(feuchte + " % RF");
    // // display.display(); // auftrag ausfuehren
    // display.setCursor(1, 50);
    // display.println(druck + "  hPa");
    // display.display(); // auftrag ausfuehren
}
