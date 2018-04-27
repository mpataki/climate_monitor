#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <secrets.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const byte ledPin = 0; // Pin with LED on Adafruit Huzzah

void callback(char* topic, byte* payload, unsigned int length) {
 Serial.print("Message arrived [");
 Serial.print(topic);
 Serial.print("] ");

 for (unsigned int i=0; i<length; i++) {
  char receivedChar = (char)payload[i];
  Serial.print(receivedChar);
  if (receivedChar == '0')
    // ESP8266 Huzzah outputs are "reversed"
    digitalWrite(ledPin, HIGH);

  if (receivedChar == '1')
    digitalWrite(ledPin, LOW);
  }
  Serial.println();
}

void ensureWiFiConnection() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi");
    int status = WiFi.begin(SSID, WIFI_PASSWORD);

    if (status != WL_CONNECTED) {
      Serial.print("failed to connect to WiFi, status: ");
      Serial.print(status);
      Serial.println(", retrying in 5 seconds");

      delay(5000);
    }
  }
}

void ensureMQTTConnection() {
 while (!mqttClient.connected()) {
   Serial.print("Attempting MQTT connection...");

    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");

      mqttClient.subscribe("ledStatus");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(", retrying in 5 seconds");

      delay(5000);
    }
  }
}

char* getTemperature() {
  return "20"; // TODO
}

char* getHumidity() {
  return "20"; // TODO
}

void sendStateUpdate() {
  StaticJsonBuffer<256> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  root["temperature"] = getTemperature();
  root["humidity"] = getHumidity();

  char payload[256];
  root.printTo(payload, sizeof(payload));

  root.prettyPrintTo(Serial);
  mqttClient.publish(MQTT_TOPIC, payload);
  delay(30000);
}

void reconnect() {
  ensureWiFiConnection();
  ensureMQTTConnection();
}

void setup()
{
 Serial.begin(9600);

 mqttClient.setServer(MQTT_SERVER, 1883);
 mqttClient.setCallback(callback);

 pinMode(ledPin, OUTPUT);
}

void loop()
{
  if (!mqttClient.connected())
    reconnect();

  sendStateUpdate();

  mqttClient.loop();
}
