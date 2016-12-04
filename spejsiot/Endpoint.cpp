#include <SpejsNode.h>
#include <Endpoint.h>

void Endpoint::bind(String _name, SpejsNode* _parent) {
    parent = _parent;
    name = _name;
}

void Endpoint::notify(String property, String value) {
    if(parent)
        parent->notify(name + "/" + property, value);
}

EndpointResult OutputEndpoint::onValue(String property, String value) {
    if (value == "1" or value == "on" or value == "true") {
        currentValue = 1;
    } else if (value == "0" or value == "off" or value == "false") {
        currentValue = 0;
    } else {
        return 400;
    }

    digitalWrite(pin, inverted ^ currentValue);
    notify("on", currentValue ? "true" : "false");
    return 200;
}

void DHTEndpoint::bind(String _name, SpejsNode* _parent) {
    Endpoint::bind(_name, _parent);

    sensor.begin();
    samplingTimer.initializeMs(samplingRate, TimerDelegate(&DHTEndpoint::sample, this)).start();
}

void DHTEndpoint::sample() {
    TempAndHumidity th;
    if(sensor.readTempAndHumidity(th))
    {
        notify("degree", String(th.temp));
        notify("humidity", String(th.humid));
    }
    else
    {
        Serial.print("Failed to read from DHT: ");
        Serial.print(sensor.getLastError());
    }
}

EndpointResult ImplementationEndpoint::onValue(String property, String value) {
    if (property == "ota" and value == "true") {
        startOTA();
        return 200;
    }
    return 400;
}

void ImplementationEndpoint::startOTA() {
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

    otaUpdater->setCallback(otaUpdateDelegate(&ImplementationEndpoint::otaUpdateCallback, this));
    otaUpdater->start();

    notify("ota", "started");
}

void ImplementationEndpoint::otaUpdateCallback(bool result) {
    if(result == true) {
        // success
        notify("ota", "finished");

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
        notify("ota", "failed");
    }
}
