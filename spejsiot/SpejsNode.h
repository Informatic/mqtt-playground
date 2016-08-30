#include <user_config.h>
#include <common_config.h>
#include <SmingCore/SmingCore.h>

class SpejsNode {
protected:
    String deviceID;
    String deviceType;

    MqttClient mqtt;

    Timer keepaliveTimer;

    rBootHttpUpdate* otaUpdater = 0;

    HashMap<String, MqttStringSubscriptionCallback> inputs;

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
    void registerInput(String key, MqttStringSubscriptionCallback cb);
    void mqttCallback(String, String);
    void controlHandler(String, String);
    void otaUpdateCallback(bool result);
};
