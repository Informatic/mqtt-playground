#ifndef SPEJSNODE_H
#define SPEJSNODE_H

#include <user_config.h>
#include <common_config.h>
#include <SmingCore/SmingCore.h>
#include <Endpoint.h>

class SpejsNode {
protected:
    String deviceID;
    String deviceType;

    MqttClient mqtt;
    HttpServer http;

    Timer keepaliveTimer;

    rBootHttpUpdate* otaUpdater = 0;

    HashMap<String, Endpoint*> endpoints;

    void onConnected();
    void startOTA();
    void keepAliveHandler();

    uint8_t currentSlot;

public:
    SpejsNode(String _deviceType) :
        mqtt(MQTT_BROKER, MQTT_PORT, MqttStringSubscriptionCallback(&SpejsNode::mqttCallback, this)),
        deviceType(_deviceType)
    {};

    void init();

    bool notify(String key, String value);
    void registerEndpoint(String key, Endpoint* cb);
    void mqttCallback(String, String);
    void controlHandler(String, String);
    void otaUpdateCallback(rBootHttpUpdate& updater, bool result);
    void httpFile(HttpRequest &request, HttpResponse &response);
    void httpIndex(HttpRequest &request, HttpResponse &response);
    void httpMetadata(HttpRequest &request, HttpResponse &response);
};

#endif
