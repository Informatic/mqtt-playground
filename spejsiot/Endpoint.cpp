#include <SpejsNode.h>
#include <Endpoint.h>

void Endpoint::bind(String key, SpejsNode* _parent) {
    parent = _parent;
}

void Endpoint::notify(String value) {
    /*if(parent)
        parent->notify(this, value);*/
}

template <class T> void InputEndpoint<T>::updateValue(T newValue) {
    value = newValue;
    //if(parent)
    //    parent->notify(String("xD"), String(value));
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
