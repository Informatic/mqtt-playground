#include <user_config.h>
#include <common_config.h>
#include <SmingCore/SmingCore.h>

typedef void (*InputCallback)(String);

class SpejsNode {
protected:
    String deviceID;
    String deviceType;
    MqttClient mqtt; // (MQTT_BROKER, MQTT_PORT);
    Timer keepaliveTimer;

    HashMap<String, InputCallback> inputs;

    void onConnected();
    void startOTA();
    void keepAliveHandler();

public:
    SpejsNode(String _deviceType) :
        mqtt(MQTT_BROKER, MQTT_PORT, MqttStringSubscriptionCallback(&SpejsNode::mqttCallback, this)),
        //*[](String topic, String message) {
        //    Serial.printf("*** message received @ %s:\n\t%s\n***\n", topic.c_str(), message.c_str());
        //}),
        deviceType(_deviceType)
    {};

    void init();

    //void registerInput(uint32_t gpio);
    //void registerOutput(uint32_t gpio);

    bool notify(String key, String value);
    void registerInput(String key, InputCallback cb);
    void mqttCallback(String, String);
};

/*
MqttClient mqtt(MQTT_BROKER, MQTT_PORT, *[](String topic, String message) {
	Serial.printf("*** message received @ %s:\n\t%s\n***\n", topic.c_str(), message.c_str());
});

Timer keepaliveTimer;
String deviceName;

void startMqttClient()
{
	Serial.println("*** Connecting to MQTT as " + deviceName);

	mqtt.setWill("main/status/" + deviceName, "offline", 1, true);
	mqtt.connect(deviceName);
	mqtt.publish("main/status/" + deviceName, "online");

	keepaliveTimer.initializeMs(5000, *[] {
		mqtt.publish("main/status/" + deviceName, "alive " + String(millis()));
		}).start();
}

void init()
{
	deviceName = "switch-" + WifiStation.getMAC().substring(6, 12);

	Serial.begin(SERIAL_BAUD_RATE);  // 115200 by default
	Serial.systemDebugOutput(false); // Debug output to serial

	Serial.println("*** Starting " + deviceName + " ...");

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.setIP(IPAddress(10, 5, 0, 39), IPAddress(255, 255, 255, 0), IPAddress(10, 5, 0, 1));
	WifiStation.enable(true);

	WifiAccessPoint.enable(false);


	WifiStation.waitConnection(*[] {
		Serial.println("*** Connection succeeded");
		startMqttClient();
		}, 20, *[] {
		Serial.println("*** Connection failed");
		});

	attachInterrupt(BTN_PIN, *[] {
		static int lastSwitch = 0;

		// Debouncing
		if(lastSwitch + 150 > millis()) {
			Serial.println("--- debouncing");
			return;
		}
		lastSwitch = millis();

		Serial.println("*** Button pressed");
		mqtt.publish("light/status", "toggle");
		}, FALLING);
}
*/
