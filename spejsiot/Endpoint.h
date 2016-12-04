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
    SpejsNode* parent;
    String name;

public:
    String type;

    Endpoint(String _type = "unknown") : type(_type) { }

    virtual void bind(String _name, SpejsNode* _parent);
    void notify(String property, String value);

    virtual EndpointResult onValue(String property, String value) {
        return 400;
    }
};

class OutputEndpoint : public Endpoint {
private:
    int pin;
    bool inverted;
    bool currentValue;

public:
    OutputEndpoint(int _pin, bool _inverted = false) : Endpoint("output"),
        pin(_pin), inverted(_inverted), currentValue(inverted) {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, currentValue);
        }

    EndpointResult onValue(String property, String value);
};

template <class T> class ValueEndpoint : public Endpoint {
protected:
    T value;
    void updateValue(T newValue);

public:
    ValueEndpoint(String _type) : Endpoint(_type) {}
};

#include <Libraries/DHT/DHT.h>

class DHTEndpoint : public ValueEndpoint<float> {
private:
    DHT sensor;
    Timer samplingTimer;
    int samplingRate;

protected:
    void sample();

public:
    DHTEndpoint(int _pin, int _samplingRate = 10000) :
        ValueEndpoint("dht"), sensor(_pin, DHT11), samplingRate(_samplingRate) {}

    void bind(String name, SpejsNode* _parent);
};

class ImplementationEndpoint : public Endpoint {
protected:
    rBootHttpUpdate* otaUpdater = 0;

public:
    ImplementationEndpoint() : Endpoint("$implementation") {}

    EndpointResult onValue(String property, String value);
    void otaUpdateCallback(bool result);
    void startOTA();
};

#endif
