#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <secrets.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 0       // Pin which is connected to the DHT sensor.
#define DHTTYPE DHT22  // AM2302

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t minSensorDelay; // ms
uint32_t coreLoopDelay = 30000;

const byte ledPin = 0; // Pin with LED on Adafruit Huzzah

void ensureWiFiConnection() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi");
    int status = WiFi.begin(SSID, WIFI_PASSWORD);

    if (status != WL_CONNECTED) {
      Serial.print("failed to connect to WiFi, status: ");
      Serial.print(status);
      Serial.println(", retrying in 5 seconds");

      delay(coreLoopDelay);
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

float getTemperature() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);

  if (isnan(event.temperature)) {
    Serial.println("Error while reading temperature");
    return 0.0f;
  }

  return event.temperature; //C
}

float getHumidity() {
  sensors_event_t event;
  dht.humidity().getEvent(&event);

  if (isnan(event.relative_humidity)) {
    Serial.println("Error while reading humidity");
    return 0.0f;
  }

  return event.relative_humidity; // %
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
}

void reconnect() {
  ensureWiFiConnection();
  ensureMQTTConnection();
}

void setup()
{
  Serial.begin(9600);
  mqttClient.setServer(MQTT_SERVER, 1883);

  Serial.println("Initializing Sensor");
  dht.begin();

  Serial.println("Sensor Information:");
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");
  Serial.println("------------------------------------");

  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");
  Serial.println("------------------------------------");

  // Set delay between sensor readings based on sensor details.
  minSensorDelay = sensor.min_delay / 1000; // ms
}

uint32_t loopDelay() {
  return max(coreLoopDelay, minSensorDelay);
}

void loop()
{
  if (!mqttClient.connected())
    reconnect();

  sendStateUpdate();

  mqttClient.loop();

  delay(loopDelay());
}
