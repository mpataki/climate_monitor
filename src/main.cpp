// #include <EEPROM.h>
#include <PubSubClient.h>
#include <secrets.h>
#include "Core.h"
#include "ClimateSensor.h"

Core core;
ClimateSensor climateSensor;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void sendStateUpdate() {
  StaticJsonBuffer<256> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  root["temperature"] = climateSensor.getTemperature();
  root["humidity"] = climateSensor.getHumidity();

  char payload[256];
  root.printTo(payload, sizeof(payload));

  root.prettyPrintTo(Serial);
  mqttClient.publish(MQTT_TOPIC, payload); // TODO: MQTT_TOPIC should be configurable
}

bool ensureMqttConnected() {
  if (mqttClient.connected()) return true;

  int retryLimit = 5;
  int tries = 1;

  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) { // TODO: all of these should be configurable
      Serial.println("MQTT connection established");
    } else if (tries < retryLimit) {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(", retrying in 5 seconds");

      delay(5000);
      tries++;
    } else {
      return false;
    }
  }

  return true;
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting up module");

  core.setup();
  // climateSensor.setup();

  mqttClient.setServer(core.getMqttServerAddress(), core.getMqttServerPort());
}

void loop()
{
  Serial.println("starting loop iteration");
  ensureMqttConnected();

  //sendStateUpdate();
  mqttClient.loop();

  // should use delay vs some sort of ESP level sleep to reduce power consumption?
  delay(max(core.getCoreLoopDelay(), climateSensor.getMinSensorScanInterval()));
}
