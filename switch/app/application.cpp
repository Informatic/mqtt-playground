#include <SpejsNode.h>

SpejsNode node("switch");

void init() {
	node.init();
	node.registerInput("relay", *[](String key, String value) {
		Serial.println("handler");
		digitalWrite(5, value == "1" ? HIGH : LOW);
		});
	pinMode(5, OUTPUT);
	digitalWrite(5, LOW);
	attachInterrupt(BTN_PIN, *[] {
		node.notify("btn", String(digitalRead(BTN_PIN)));
		}, CHANGE);
	//node.registerInputGPIO("btn", BTN_PIN);
	//node.registerOutputGPIO("led", LED_PIN);
}
