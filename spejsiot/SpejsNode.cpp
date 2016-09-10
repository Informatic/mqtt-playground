#include <SpejsNode.h>

void SpejsNode::init() {
    deviceID = WifiStation.getMAC().substring(6, 12);

    currentSlot = 0;
    if(!rboot_get_last_boot_rom(&currentSlot)) {
	currentSlot = rboot_get_current_rom();
    }

    Serial.begin(115200);
    Serial.systemDebugOutput(false); // Debug output to serial
    Serial.print("*** SpejsNode init, running on:");
    Serial.print(deviceID);
    Serial.printf(", current rom: %d\r\n", currentSlot);

    WifiStation.config(WIFI_SSID, WIFI_PWD);
    WifiStation.enable(true);

    WifiAccessPoint.enable(false);

    WifiStation.waitConnection(
	ConnectionDelegate(&SpejsNode::onConnected, this),
	20, *[] {
	Serial.println("Connection failed");
	});

    inputs["control"] = MqttStringSubscriptionCallback(&SpejsNode::controlHandler, this);
}

void SpejsNode::keepAliveHandler() {
    if(mqtt.getConnectionState() != eTCS_Connected) {
        Serial.println("Reconnecting");
	onConnected();
    } else {
	uint8_t mode;
	if(rboot_get_last_boot_mode(&mode)) {
	    if(mode == MODE_TEMP_ROM) {
		rboot_set_current_rom(currentSlot);
		Serial.println("Successfuly connected, accepting temp rom");
	    } else {
		Serial.printf("Not a TEMP ROM boot: %d\r\n", mode);
	    }
	} else {
	    Serial.println("No boot mode info");
	}
    }
}

void SpejsNode::onConnected() {
    Serial.println("Connection successful");

    mqtt.setWill(TOPIC_PREFIX + deviceID + "/state", "offline", 1, true);

#ifdef ENABLE_SSL
    const uint8_t sha1Fingerprint[] = SSL_FINGERPRINT;
    mqtt.connect("iot-" + deviceID, "", "", true);
    mqtt.addSslOptions(SSL_SERVER_VERIFY_LATER);
    mqtt.setSslFingerprint(sha1Fingerprint, 20);
#else
    mqtt.connect("iot-" + deviceID);
#endif

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

void SpejsNode::registerInput(String key, MqttStringSubscriptionCallback callback) {
    inputs[key] = callback;
}

void SpejsNode::mqttCallback(String origtopic, String value) {
    String devicePrefix = TOPIC_PREFIX + deviceID;

    if(!origtopic.startsWith(devicePrefix)) {
        Serial.println("ignoring");
        return;
    }

    String topic = origtopic.substring(devicePrefix.length() + 1);

    Serial.println(topic);
    Serial.println(value);

    if(inputs.contains(topic)) {
        inputs[topic](origtopic, value);
    } else {
        Serial.println("unknown topic?");
    }
}

void SpejsNode::controlHandler(String key, String value) {
    Serial.println("Control command: " + value);
    if(value == "ota") {
        startOTA();
    } else if(value == "restart") {
        System.restart();
    } else {
        Serial.println("Invalid command");
    }
}

void SpejsNode::startOTA() {
    uint8_t slot;
    rboot_config bootconf;
    String romURL = OTA_URL + deviceID + "/rom0.bin";
    String spiffsURL = OTA_URL + deviceID + "/spiff_rom.bin";

    Serial.println("Updating...");

    // need a clean object, otherwise if run before and failed will not run again
    if (otaUpdater) delete otaUpdater;
    otaUpdater = new rBootHttpUpdate();

    bootconf = rboot_get_config();

    if (currentSlot == 0)
        slot = 1;
    else
        slot = 0;

    Serial.printf("Updating to rom %d.\r\n", slot);

    // flash rom to position indicated in the rBoot config rom table
    otaUpdater->addItem(bootconf.roms[slot], romURL);

#ifndef DISABLE_SPIFFS
    // use user supplied values (defaults for 4mb flash in makefile)
    if (slot == 0) {
	    otaUpdater->addItem(RBOOT_SPIFFS_0, spiffsURL);
    } else {
	    otaUpdater->addItem(RBOOT_SPIFFS_1, spiffsURL);
    }
#endif

    otaUpdater->setCallback(otaUpdateDelegate(&SpejsNode::otaUpdateCallback, this));
    otaUpdater->start();

    notify("ota", "started");
}

void SpejsNode::otaUpdateCallback(rBootHttpUpdate& updater, bool result) {
    if(result == true) {
	    // success
	    notify("ota", "finished");

	    uint8 slot;

	    if (currentSlot == 0)
            slot = 1;
	    else
            slot = 0;

	    // set to boot new rom and then reboot
	    Serial.printf("Firmware updated, rebooting to rom %d...\r\n", slot);

	    rboot_set_temp_rom(slot);

		System.restart();
    } else {
	    notify("ota", "failed");
    }
}
