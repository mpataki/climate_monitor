#include "Core.h"

Core::Core() :
  wifiServer(80),
  customMqttServerParam("mqtt_server", "mqtt server", "mqtt_server", 40),
  customMqttPortParam("mqtt_port", "mqtt port", "1883", 6),
  customCoreLoopDelayParam("core_loop_delay", "core loop delay", "60000", 6)
{
}

Core::~Core() {}

bool shouldSaveConfig = false;

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void Core::setup() {
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&customMqttServerParam);
  wifiManager.addParameter(&customMqttPortParam);
  wifiManager.addParameter(&customCoreLoopDelayParam);

  readConfigFile();

  // AP timeout
  wifiManager.setTimeout(120);

  if (!wifiManager.autoConnect()) { // use password here?
    Serial.println("Failed to connect to wifi. Trying again shortly.");

    // random delay to stagger reconnects from multiple devices
    //   when the wifi goes down system wide.
    delay(3000 + ESP8266TrueRandom.random(4000));
    ESP.reset();
  }

  Serial.println("Connected to wifi");

  if (shouldSaveConfig) saveConfig();
}

void Core::saveConfig() {
  Serial.println("saving config");

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  strcpy(mqttServerAddress, customMqttServerParam.getValue());
  strcpy(mqttServerPort, customMqttPortParam.getValue());
  strcpy(coreLoopDelay, customCoreLoopDelayParam.getValue());

  json["mqtt_server"] = mqttServerAddress;
  json["mqtt_port"] = mqttServerPort;
  json["core_loop_delay"] = coreLoopDelay;

  File configFile = SPIFFS.open("/config.json", "w");

  if (configFile) {
    json.printTo(configFile);
  } else {
    Serial.println("failed to open config file for writing");
  }

  json.printTo(Serial);
  configFile.close();
}

const char* Core::getMqttServerAddress() {
  return mqttServerAddress;
}

uint16_t Core::getMqttServerPort() {
  return atoi(mqttServerPort);
}

uint32_t Core::getCoreLoopDelay() {
   return atoi(coreLoopDelay);
}

void Core::readConfigFile() {
  Serial.println("mounting file system...");

  if (!SPIFFS.begin()) {
    Serial.println("failed to mount file system");
    return;
  }

  Serial.println("mounted file system");

  if (!SPIFFS.exists("/config.json")) {
    Serial.println("config.json doesn't exist");
    return;
  }

  //file exists, reading and loading
  Serial.println("reading config file");
  File configFile = SPIFFS.open("/config.json", "r");

  if (!configFile) {
    Serial.println("couldn't read config.json");
    return;
  }

  Serial.println("opened config file");
  size_t size = configFile.size();
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  json.printTo(Serial);

  if (json.success()) {
    Serial.println("\nparsed json");

    // TODO: would be great to leave these up to the consumer
    strcpy(mqttServerAddress, json["mqtt_server"]);
    strcpy(mqttServerPort, json["mqtt_port"]);
    strcpy(coreLoopDelay, json["core_loop_delay"]);

  } else {
    Serial.println("\nfailed to load json config");
  }

  configFile.close();
}
