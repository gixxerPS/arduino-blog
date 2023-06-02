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
 */
#include <Wire.h> // fuer I2C
#include <ESP8266WiFi.h> // fuer WIFI ausschalten
#include <Adafruit_BME280.h>

// lcd display include
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

Adafruit_BME280 bme1;
float temp1(NAN), hum1(NAN), press1(NAN);

bool bme1SensorError = false; // bme sensor nicht vorhanden oder nicht ok?
bool displayError = false; // display initialisierung fehlgeschlagen?

// [ms] wie lange bleibt die anzeige an nach aktivierung ?
const unsigned long MS_ANZEIGEDAUER = 5000; 

// [us] wie lange darf ich schlafen bevor ich wieder messe ?
const uint64_t US_SCHLAFINTERVAL = 10e6;

// lcd defines
#define OLED_RESET -1
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306* displayPtr = NULL;
/* ACHTUNG: der display konstruktor allokiert dynamischen speicher,
 * da das display aber zur laufzeit an und ausgeschaltet werden soll
 * um strom zu sparen muss der dynamische speicher wieder freigegeben
 * werden. da die implementierung in der bibliothek fix ist muessen 
 * wir es bei uns loesen und da sind die new / delete operatoren
 * die einzige moeglichkeit die ich aktuell sehe. */

#define PIN_SCREEN_POWER D7

void initDisplay(bool periphBegin = false) {
    digitalWrite(PIN_SCREEN_POWER, HIGH); // display einschalten
    delay(100); // etwas zeit geben zum spannung stabilisieren (WICHTIG)
                // etwas grosszuegig, aber wir habens ja nicht eilig :)
    if (displayPtr) {
        Serial.println("display objekt erst loeschen");
        delete displayPtr;
        displayPtr = NULL;
    }
    if (!displayPtr) {
        Serial.println("display objekt erfolgreich geloescht");
    }
    
    displayPtr = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

    if (!displayPtr->begin(SSD1306_SWITCHCAPVCC, 0x3C, /*reset*/ true, /*periphBegin*/ periphBegin)){
        Serial.println("display init error!");
        displayError = true;
        return;
    }
    displayPtr->setTextSize(2);
    displayPtr->setTextColor(WHITE); // ohne Effekt -> das eingesetzte Display kann nur blau
    displayPtr->clearDisplay();
    displayPtr->setCursor(5, 0);
    displayPtr->setTextSize(2);
    displayPtr->println("Init");
    displayPtr->display(); // Text zeigen
}


void setup()
{
    Serial.begin(9600); // optional: aber praktisch fuer debug ausgaben ;)
    // while (!Serial) ; 
    delay(200);
    Serial.print("compiled: "); // kompilier zeitstempel ausgeben
    Serial.print(__DATE__);
    Serial.println(__TIME__);
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(PIN_SCREEN_POWER, OUTPUT); // spannungsversorgung vom display
    digitalWrite(PIN_SCREEN_POWER, HIGH); // display einschalten

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
    if (!displayPtr) {
        Serial.println("init display");
        displayPtr = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    } else {
        Serial.println("error display ptr not NULL");
    }

    initDisplay(true);
    delay(200);
    displayPtr->clearDisplay();
    if (bme1SensorError) {
        displayPtr->setCursor(5, 0);
        displayPtr->println("Sensor-");
        displayPtr->setCursor(5, 30);
        displayPtr->print("fehler");
    } else {
        displayPtr->setCursor(5, 0);
        displayPtr->println("Hallo");
        displayPtr->setTextSize(2);
        displayPtr->setCursor(5, 30);
        displayPtr->print("Datenlogger");
    }
    displayPtr->display(); // Text zeigen
    WiFi.mode( WIFI_OFF );

    //digitalWrite(PIN_SCREEN_POWER, LOW); // display ausschalten -> strom sparen
    WiFi.forceSleepBegin(); 

    //ESP.deepSleep(10e6); // [us]
}

void loop()
{
    static unsigned long millisOld = 0;
    static unsigned long millisLastRead = 0; // letztes sensor auslesen
    unsigned long curMillis = millis();

    if  (curMillis - millisOld > 1000) {
        // led toggeln / invertieren => lebenszeichen
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); 
        millisOld = curMillis;
    }

    // anzeigezeit abgelaufen ?
    if (curMillis > MS_ANZEIGEDAUER) {
        // dann display ausschalten
        digitalWrite(PIN_SCREEN_POWER, LOW);
        Serial.println("stoppe anzeige. lege mich schlafen...");
        ESP.deepSleep(US_SCHLAFINTERVAL); // [us]

    }
    // anzeigezeit laeuft ? 
    else {
        // dann alle 1s werte anzeigen
        if (!bme1SensorError) {
            if (curMillis - millisLastRead > 1000) {
                readAndPrintBME280Data();
                millisLastRead = curMillis;
            }
        }
    }
}

void readAndPrintBME280Data()
{
    temp1 = bme1.readTemperature();
    hum1 = bme1.readHumidity();
    press1 = bme1.readPressure() / 100;

    String Temperatur = String(temp1);
    Serial.print("Temp = " + Temperatur + " C | ");

    String feuchte = String(hum1);
    Serial.print("Feuchte = " + feuchte + " %RF | ");

    // ohne nachkommastellen darstellen und dabei richtig runden
    String druck = String(int(press1 + 0.5f));
    Serial.println("Druck = " + druck + " hPa");

    // lcd
    displayPtr->clearDisplay();
    displayPtr->setTextSize(2);
    displayPtr->setTextColor(WHITE);
    displayPtr->setCursor(1, 1);
    displayPtr->println(Temperatur + " C");

    displayPtr->setCursor(1, 25);
    displayPtr->println(feuchte + " % RF");

    displayPtr->setCursor(1, 50);
    displayPtr->println(druck + "  hPa");
    displayPtr->display(); // anzeige auftrag ausfuehren
}
