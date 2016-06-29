#include <SpejsNode.h>

void SpejsNode::init() {
    deviceID = WifiStation.getMAC().substring(6, 12);

    Serial.begin(115200);
    Serial.systemDebugOutput(false); // Debug output to serial
    Serial.print("*** SpejsNode init, running on: ");
    Serial.println(deviceID);

    WifiStation.config(WIFI_SSID, WIFI_PWD);
    WifiStation.enable(true);

    WifiAccessPoint.enable(false);

    WifiStation.waitConnection(
	ConnectionDelegate(&SpejsNode::onConnected, this),
	20, *[] {
	Serial.println("Connection failed");
	});
}

void SpejsNode::keepAliveHandler() {
    if(mqtt.getConnectionState() != eTCS_Connected) {
        Serial.println("Reconnecting");
	onConnected();
    }
}

void SpejsNode::onConnected() {
    Serial.println("Connection successful");
    // "+deviceID+"/
    mqtt.setWill(TOPIC_PREFIX + deviceID + "/state", "offline", 1, true);
    mqtt.connect("iot-" + deviceID);

    mqtt.subscribe(TOPIC_PREFIX + deviceID + "/control");

    for(unsigned int i = 0 ; i < inputs.count() ; i++) {
	mqtt.subscribe(TOPIC_PREFIX + deviceID + "/" + inputs.keyAt(i));
    }

    mqtt.publish(TOPIC_PREFIX + deviceID + "/state", "online");
    mqtt.publish(TOPIC_PREFIX + deviceID + "/type", deviceType);

    keepaliveTimer.initializeMs(10000, TimerDelegate(&SpejsNode::keepAliveHandler, this)).start();
}

bool SpejsNode::notify(String key, String value) {
    if(mqtt.getConnectionState() == eTCS_Connected) {
        mqtt.publish("iot/" + deviceID + "/" + key, value);
        return true;
    }

    return false;
}

void SpejsNode::registerInput(String key, InputCallback callback) {
    inputs[key] = callback;
}

void SpejsNode::mqttCallback(String topic, String value) {
    String devicePrefix = TOPIC_PREFIX + deviceID;
    if(!topic.startsWith(devicePrefix)) {
	Serial.println("ignoring");
	return;
    }

    topic = topic.substring(devicePrefix.length() + 1);

    Serial.println(topic);
    Serial.println(value);

    if(inputs.contains(topic)) {
	Serial.println("dupa");
	inputs[topic](value);
    } else {
	Serial.println("default");
    }
}
