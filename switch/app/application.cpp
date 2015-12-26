#include <user_config.h>
#include <common_config.h>
#include <SmingCore/SmingCore.h>

MqttClient mqtt(MQTT_BROKER, MQTT_PORT, *[](String topic, String message) {
	Serial.printf("*** message received @ %s:\n\t%s\n***\n", topic.c_str(), message.c_str());
});

void startMqttClient()
{
	String deviceName = "switch-" + WifiStation.getMAC().substring(6, 12);
	Serial.println("*** Connecting to MQTT as " + deviceName);
	mqtt.connect(deviceName);
	mqtt.subscribe("main/status/#");
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);  // 115200 by default
	Serial.systemDebugOutput(false); // Debug output to serial

	Serial.println("*** Starting ...");
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);

	WifiAccessPoint.enable(false);

	WifiStation.waitConnection(*[] {
		Serial.println("*** Connection succeeded");
		startMqttClient();
		}, 20, *[] {
		Serial.println("*** Connection failed");
		});

	attachInterrupt(BTN_PIN, *[] {
		bool btnState = digitalRead(BTN_PIN);
		Serial.printf("*** Button state: %d\n", btnState);
		mqtt.publish("btn/status", String(btnState));
		}, CHANGE);
}
