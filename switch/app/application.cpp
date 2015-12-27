#include <user_config.h>
#include <common_config.h>
#include <SmingCore/SmingCore.h>

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
