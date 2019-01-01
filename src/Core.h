
#ifndef Core_h
#define Core_h

#include <FS.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266TrueRandom.h>

class Core {
  public:
    Core();
    ~Core();
    void setup();
    const char* getMqttServerAddress();
    uint16_t getMqttServerPort();
    uint32_t getCoreLoopDelay();

    WiFiManager wifiManager;
    WiFiServer wifiServer;

  private:
    void readConfigFile();
    void saveConfig();

    char mqttServerAddress[40];
    char mqttServerPort[6];
    char coreLoopDelay[6];

    WiFiManagerParameter customMqttServerParam;
    WiFiManagerParameter customMqttPortParam;
    WiFiManagerParameter customCoreLoopDelayParam;
};

#endif
