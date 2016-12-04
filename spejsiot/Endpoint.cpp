#include <SpejsNode.h>
#include <Endpoint.h>

void Endpoint::bind(String _key, SpejsNode* _parent) {
    parent = _parent;
    key = _key;
}

void Endpoint::notify(String value) {
    if(parent)
        parent->notify(key, value);
}

template <class T> void ValueEndpoint<T>::updateValue(T newValue) {
    value = newValue;
    notify(String(value));
}

template <class T> void ValueEndpoint<T>::fillValue(JsonObject& obj) {
    obj["value"] = value;
}

EndpointResult ControlEndpoint::onValue(String key, String value) {
    if (value == "ota") {
        return startOTA();
    } else if(value == "restart") {
        System.restart();
        return 200;
    } else {
        return 400;
    }
}

EndpointResult ControlEndpoint::startOTA() {
    uint8_t slot;
    rboot_config bootconf;
    String romURL = OTA_URL + parent->deviceID + "/rom0.bin";
    String spiffsURL = OTA_URL + parent->deviceID + "/spiff_rom.bin";

    Serial.println("Updating...");

    // need a clean object, otherwise if run before and failed will not run again
    if (otaUpdater) delete otaUpdater;
    otaUpdater = new rBootHttpUpdate();

    bootconf = rboot_get_config();

    if (parent->currentSlot == 0)
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

    otaUpdater->setCallback(otaUpdateDelegate(&ControlEndpoint::otaUpdateCallback, this));
    otaUpdater->start();

    //notify("ota", "started");
    return 200;
}

void ControlEndpoint::otaUpdateCallback(bool result) {
    if(result == true) {
        // success
        //notify("ota", "finished");
        Serial.println("ota finished");

        uint8 slot;

        if (parent->currentSlot == 0)
            slot = 1;
        else
            slot = 0;

        // set to boot new rom and then reboot
        Serial.printf("Firmware updated, rebooting to rom %d...\r\n", slot);

        rboot_set_temp_rom(slot);
        System.restart();
    } else {
        Serial.println("ota failed");
        //notify("ota", "failed");
    }
}

void OutputEndpoint::fillValue(JsonObject& obj) {
    obj["value"] = currentValue;
}

EndpointResult OutputEndpoint::onValue(String key, String value) {
    if (value == "1" or value == "on") {
        currentValue = 1;
    } else if (value == "0" or value == "off") {
        currentValue = 0;
    //} else if (value == "toggle") {
    //    currentValue = !currentValue;
    } else {
        return 400;
    }

    digitalWrite(pin, inverted ^ currentValue);
    return 200;
}

void DHTEndpoint::bind(String _key, SpejsNode* _parent) {
    parent = _parent;
    key = _key;

    sensor.begin();
    samplingTimer.initializeMs(samplingRate, TimerDelegate(&DHTEndpoint::sample, this)).start();
}

void DHTEndpoint::sample() {
    TempAndHumidity th;
    if(sensor.readTempAndHumidity(th))
    {
        updateValue(th.temp);
    }
    else
    {
        Serial.print("Failed to read from DHT: ");
        Serial.print(sensor.getLastError());
    }
}
