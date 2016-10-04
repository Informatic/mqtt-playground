#include <SpejsNode.h>
#include <Endpoint.h>

void Endpoint::bind(String _key, SpejsNode* _parent) {
    parent = _parent;
    key = _key;
}

void Endpoint::notify(String value) {
    /*if(parent)
      parent->notify(this, value);*/
}

template <class T> void ValueEndpoint<T>::updateValue(T newValue) {
    value = newValue;

    // TODO parent->notify(this, String(value)) ?
    if(parent)
        parent->notify(key, String(value));
}

template <class T> void ValueEndpoint<T>::fillValue(JsonObject& obj) {
    obj["value"] = value;
}

EndpointResult ControlEndpoint::onValue(String key, String value) {
    // TODO

    if (value == "ota") {
        return 200;
    } else if(value == "restart") {
        return 200;
    } else {
        return 400;
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
    } else if (value == "toggle") {
        currentValue = !currentValue;
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
