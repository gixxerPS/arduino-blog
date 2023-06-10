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
 * - Temperatursensor BME280
 *     D1 SCL
 *     D2 SCA
 *     6 polige BME Variante versorgt mit 3,3V
 *     CSB = 3,3V (pullup)
 *     SDO = 3,3V (pullup)
 *  - OLED 128x64
 *     D1 SCL
 *     D2 SCA
 *     D4 Versorgung
 */
#include <Wire.h> // fuer I2C
#include <ESP8266WiFi.h> // fuer WIFI aus- / einschalten
#include <Adafruit_BME280.h>

// lcd display include
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include <SD.h> // fuer SD karte
#include <RtcDS1307.h> // rtc include https://github.com/Makuna/Rtc/blob/master/examples/DS1307_Simple/DS1307_Simple.ino

// fuer sd
#define CS D8

RtcDS1307<TwoWire> Rtc(Wire);
RtcDateTime now;

Adafruit_BME280 bme1;
float temp1(NAN), hum1(NAN), press1(NAN);

bool SDKarteError = false;    // sd karte nicht vorhanden oder nicht ok?
bool RTCError = false;        // rtc nicht vorhanden oder nicht ok? 
bool bme1SensorError = false; // bme sensor nicht vorhanden oder nicht ok?
bool displayError = false; // display initialisierung fehlgeschlagen?

// [ms] wie lange bleibt die anzeige an nach aktivierung ?
const unsigned long MS_ANZEIGEDAUER = 10000; 

// [us] wie lange darf ich schlafen bevor ich wieder messe ?
const uint64_t US_SCHLAFINTERVAL = 120e6;

const String NAME_LOGDATEI = "loggerDaten";
const String NAME_LOGDATEI_CSV = NAME_LOGDATEI + String(".csv");

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

// Achtung: D4 ist auch der Pin der User LED beim D1 mini
#define PIN_SCREEN_POWER D4

#define countof(a) (sizeof(a) / sizeof(a[0]))

/**
 * diese setup funktion koennte man dynamisch zur Laufzeit aufrufen.
 * damit koennte das display ein-/ausgeschaltet werden ohne dass
 * der controller neugestartet / geresettet werden muss. 
 * in diesem fall muesste es mit periphBegin = false aufgerufen werden
 * um das einfrieren durch doppelten aufruf von Wire.begin() 
 * zu vermeiden. */
void setupDisplay(bool periphBegin = false) {
    digitalWrite(PIN_SCREEN_POWER, HIGH); // display einschalten
    delay(100); // etwas zeit geben zum spannung stabilisieren (WICHTIG)
                // etwas grosszuegig, aber wir habens ja nicht eilig :)
    if (displayPtr) {
        delete displayPtr;
        displayPtr = NULL;
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

void setupSd() 
{
    Serial.print("Initialisiere SD-Karte...");
    if (!SD.begin(CS)) {
        Serial.println(" fehlgeschlagen!");
        SDKarteError = true;
    }
    Serial.println(" fertig.");
    // Datei mit Schreibzugriff oeffnen bzw erzeugen wenn nicht vorhanden.
    File zielDatei = SD.open(NAME_LOGDATEI + String("_beschreibung.txt"), FILE_WRITE);
    
    if (zielDatei) { // oeffnen erfolgreich?
        Serial.print("Datei gefunden.");
        // Spaltenueberschrift eintragen als beschreibung fuer csv datei
        zielDatei.println("Datum;Uhrzeit;Temperatur;Feuchte;Druck;");
        zielDatei.close();
    } else { // fehler beim Zugriff die Datei...
        Serial.println("Fehler beim Oeffnen der Datei auf SD Karte :(");
        SDKarteError = true;
    }
}

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];
    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u.%02u.%04u %02u:%02u:%02u"),
            dt.Day(),
            dt.Month(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);
}

void setupRTC() 
{
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    Serial.print("Kompilierzeitstempel: ");
    printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid()) {
        if (Rtc.LastError() != 0) {
            // we have a communications error
            // see https://www.arduino.cc/en/Reference/WireEndTransmission for
            // what the number means
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
            RTCError = true;
        } else  {
            // Common Causes:
            //    1) first time you ran and the device wasn't running yet
            //    2) the battery on the device is low or even missing
            Serial.println("RTC lost confidence in the DateTime!");
            // following line sets the RTC to the date & time this sketch was compiled
            // it will also reset the valid flag internally unless the Rtc device is
            // having an issue
            Rtc.SetDateTime(compiled);
        }
    }
    if (!Rtc.GetIsRunning()) {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }
    now = Rtc.GetDateTime();
    if (now < compiled) {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.SetSquareWavePin(DS1307SquareWaveOut_Low);
}

void setupBME280() 
{
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
}

void setup()
{
    Serial.begin(9600); // optional: aber praktisch fuer debug ausgaben ;)
    while (!Serial) ; 
    delay(200);
    Serial.println(">>>>>>>>>>>>>>>>>>>> Starte Datenlogger...");

    pinMode(PIN_SCREEN_POWER, OUTPUT); // spannungsversorgung vom display
    digitalWrite(PIN_SCREEN_POWER, HIGH); // display einschalten

    setupBME280();
    setupSd();
    setupRTC();
    setupDisplay(true);
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
    WiFi.forceSleepBegin(); 
}

void readRTC() {
    if (!Rtc.IsDateTimeValid()) {
        if (Rtc.LastError() != 0) {
            // we have a communications error
            // see https://www.arduino.cc/en/Reference/WireEndTransmission for 
            // what the number means
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
            RTCError = true;
        }
        else {
            // Common Causes:
            //    1) the battery on the device is low or even missing and the power line was disconnected
            Serial.println("RTC lost confidence in the DateTime!");
        }
    }
    now = Rtc.GetDateTime();
}

void writeDataToSD() {
// Zeilenaufbau CSV Datei (comma-sperated-value) => excel format
// Datum;Uhrzeit;Temperatur;Feuchte;Druck;
// 01.09.22;18:10:23;23.2;41.23;1023.22;
  char buffer[80];

  sprintf(buffer, "%02d.%02d.%d;%d:%d:%d;%4.1f;%.0f;%.0f;", 
      now.Day(), now.Month(), now.Year(),
      now.Hour(), now.Minute(), now.Second(),
      temp1, hum1, press1);
    Serial.println(buffer);

    File zielDatei = SD.open(NAME_LOGDATEI_CSV, FILE_WRITE);
    
    if (zielDatei) { //existiert die Datei ?
        zielDatei.println(buffer);
        zielDatei.close();
    } else { // Dateifehler
        Serial.print("Fehler beim Schreiben auf SD-Karte");  
    }
}

void loop()
{
    static unsigned long millisLastRead = 0; // letztes sensor auslesen
    unsigned long curMillis = millis();

    // anzeigezeit abgelaufen ?
    if (curMillis > MS_ANZEIGEDAUER) {
        // dann display ausschalten
        //digitalWrite(PIN_SCREEN_POWER, LOW);
        Serial.println("stoppe anzeige. lege mich schlafen...");
        ESP.deepSleep(US_SCHLAFINTERVAL); // [us]
    } else { // anzeigezeit laeuft ? 
        // dann alle 2s werte anzeigen/schreiben
        if (curMillis - millisLastRead > 2000) {
            readRTC();
            if (!RTCError) {
                printDateTime(now); // RTC Zeit
                Serial.print(" | ");
            }
            if (!bme1SensorError) {
                readAndPrintBME280Data();
            }
            if (!RTCError && !bme1SensorError && !SDKarteError) {
                writeDataToSD();
            }
            millisLastRead = curMillis;
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
