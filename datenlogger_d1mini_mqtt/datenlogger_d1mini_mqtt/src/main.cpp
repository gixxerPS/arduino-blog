/*******************************************************************************
 * @file main.cpp
 * @brief MQTT publish / subcribe datalogger.
 *
 * Tasks:
 * - login to wifi
 * - connect to mqtt server
 * - subscribe to command topic
 * - wait till command to publish data is received
 * - publish (sensor) data
 *
 ******************************************************************************/
#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/tree/master
#include "mypassword.h" // for wifi password not to be published on github :)

// Beispiel von:
// https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_esp8266/mqtt_esp8266.ino

Adafruit_BME280 bme1;
float temp1(-99.9f), hum1(-88.8), press1(-77.7);

bool bme1SensorError = false; // bme sensor nicht vorhanden oder nicht ok?

const char* ssid = "GSXR750";
// password in mypassword.h
const char* MQTT_SERVER = "192.168.2.64";

String MQTT_TOPIC_BASE      = "gartenhaus"; // Beispiel fuer Station
String MQTT_TOPIC_SUBSCRIBE = MQTT_TOPIC_BASE+"/cmd"; // e.g. gartenhaus/cmd
String MQTT_TOPIC_PUBLISH   = MQTT_TOPIC_BASE+"/sensorData"; // e.g. gartenhaus/sensorData

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
bool cmdSend = false;

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
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if ((char)payload[0] == '1') { // we subscribed to the right topic so '1' => start
    digitalWrite(LED_BUILTIN, LOW);
    cmdSend = true;
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void reconnect() {
  while (!client.connected()) { // Loop until we're reconnected
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX); // Create a random client ID
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC_SUBSCRIBE.c_str()); // ... and resubscribe
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(74880);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
}

void datenSchicken() {
  String topic = MQTT_TOPIC_PUBLISH + "/temp";
  client.publish(topic.c_str(), String(temp1).c_str());
  topic = MQTT_TOPIC_PUBLISH + "/humidity";
  client.publish(topic.c_str(), String(hum1).c_str());
  topic = MQTT_TOPIC_PUBLISH + "/pressure";
  client.publish(topic.c_str(), String(press1).c_str());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (cmdSend) {
    cmdSend = false;
    datenSchicken();
  }
}