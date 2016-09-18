#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <SmingCore/SmingCore.h>

class SpejsNode;

class EndpointResult {
public:
    int status;
    String description;

    EndpointResult(int _status) : status(_status) {}
    EndpointResult(int _status, String _description) :
        status(_status), description(_description) {}
};

class Endpoint {
protected:
    const SpejsNode* parent;

public:
    String type;

    Endpoint(String _type = "unknown") : type(_type) { }

    virtual void bind(String key, SpejsNode* _parent);
    void notify(String value);

    virtual EndpointResult onValue(String key, String value) {
        return 400;
    }

    virtual void fillValue(JsonObject& obj) { }
};

class ControlEndpoint : public Endpoint {
public:
    ControlEndpoint() : Endpoint("control") {}
    EndpointResult onValue(String key, String value);
};

class OutputEndpoint : public Endpoint {
private:
    int pin;
    bool inverted;
    bool currentValue;

public:
    OutputEndpoint(int _pin, bool _inverted = false) :
        Endpoint("output"), pin(_pin), inverted(_inverted), currentValue(inverted) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, currentValue);
    }

    EndpointResult onValue(String key, String value);
    void fillValue(JsonObject& obj);
};

template <class T> class InputEndpoint : public Endpoint {
protected:
    T value;
    void updateValue(T newValue);

public:
    InputEndpoint(String _type) : Endpoint(_type) {}
};

#include <Libraries/DHT/DHT.h>

class DHT22Endpoint : public InputEndpoint<int> {
private:
    DHT sensor;
    Timer samplingTimer;
    int samplingRate;

public:
    DHT22Endpoint(int _pin, int _samplingRate = 10000) :
        InputEndpoint("dht22"), sensor(_pin), samplingRate(_samplingRate) {}
};

#endif
