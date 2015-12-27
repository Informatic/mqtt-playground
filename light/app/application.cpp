#include <user_config.h>
#include <common_config.h>
#include <SmingCore/SmingCore.h>

bool state = false;

MqttClient mqtt(MQTT_BROKER, MQTT_PORT, *[](String topic, String message) {
	Serial.printf("*** message received @ %s:\n\t%s\n***\n", topic.c_str(), message.c_str());
	if(message == "on" || message == "1") {
		state = true;
	} else if(message == "off" || message == "0") {
		state = false;
	} else if(message == "toggle") {
		state = !state;
	}

	digitalWrite(LED_PIN, !state);
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

	mqtt.subscribe("light/status");
}

void init()
{
	deviceName = "light-" + WifiStation.getMAC().substring(6, 12);

	Serial.begin(SERIAL_BAUD_RATE);  // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	Serial.println("*** Starting " + deviceName + " ...");

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);

	WifiAccessPoint.enable(false);


	WifiStation.waitConnection(*[] {
		Serial.println("*** Connection succeeded");
		startMqttClient();
		}, 20, *[] {
		Serial.println("*** Connection failed");
		});

	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, HIGH);
}
