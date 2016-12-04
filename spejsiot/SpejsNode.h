#ifndef SPEJSNODE_H
#define SPEJSNODE_H

#include <user_config.h>
#include <common_config.h>
#include <SmingCore/SmingCore.h>
#include <Endpoint.h>

#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

class SpejsNode {
protected:
    MqttClient mqtt;
    HttpServer http;

    Timer keepaliveTimer;

    HashMap<String, Endpoint*> endpoints;

    void onConnected();
    void keepAliveHandler();
    void initializeMDNS();

    void buildMetadata(JsonObjectStream* stream);

public:
    String deviceID;
    String deviceType;

    uint8_t currentSlot;

    SpejsNode(String _deviceType) :
        mqtt(MQTT_BROKER, MQTT_PORT, MqttStringSubscriptionCallback(&SpejsNode::mqttCallback, this)),
        deviceType(_deviceType) {};

    void init();

    bool notify(String key, String value);
    void registerEndpoint(String key, Endpoint* cb);
    void mqttCallback(String, String);
    void otaUpdateCallback(bool result);
    void httpFile(HttpRequest &request, HttpResponse &response);
    void httpIndex(HttpRequest &request, HttpResponse &response);
    void httpMetadata(HttpRequest &request, HttpResponse &response);
};

#endif
