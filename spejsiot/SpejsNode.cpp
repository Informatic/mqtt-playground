#include <SpejsNode.h>
#include <Endpoint.h>
#include <ver.h>

void SpejsNode::init() {
    deviceID = WifiStation.getMAC().substring(6, 12);

    currentSlot = 0;
    if(!rboot_get_last_boot_rom(&currentSlot)) {
        currentSlot = rboot_get_current_rom();
    }

    Serial.begin(115200);
    Serial.systemDebugOutput(false); // Debug output to serial
    Serial.print("*** SpejsNode init, running on: ");
    Serial.print(deviceID);
    Serial.printf(", current rom: %d\r\n", currentSlot);

    WifiAccessPoint.enable(false);
    WifiStation.enable(true);
    WifiStation.config(WIFI_SSID, WIFI_PWD);

    WifiStation.waitConnection(
        ConnectionDelegate(&SpejsNode::onConnected, this), 20, *[] {
            Serial.println("Connection failed");
        });

    registerEndpoint("control", new ControlEndpoint());
}

void SpejsNode::keepAliveHandler() {
    static int failureCounter = 0;

    if(mqtt.getConnectionState() != eTCS_Connected) {
        Serial.println("Reconnecting");
        if(failureCounter++ < 5)
            onConnected();
        else
            System.restart();
    } else {
        failureCounter = 0;

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

void SpejsNode::httpIndex(HttpRequest &request, HttpResponse &response)
{
    response.sendString("This is spejsiot device, take a look at: https://wiki.hackerspace.pl/projects:spejsiot");
}

void SpejsNode::buildMetadata(JsonObjectStream* stream)
{
    JsonObject& json = stream->getRoot();
    json["version"] = 1;
    json["device_id"] = deviceID;
    json["device_type"] = deviceType;
    json["rom_slot"] = currentSlot;
    json["rom_rev"] = BUILD_ID;

    JsonArray& endpoints_list = json.createNestedArray("endpoints");
    for(unsigned int i = 0; i < endpoints.count(); i++) {
        JsonObject& obj = endpoints_list.createNestedObject();
        obj["name"] = endpoints.keyAt(i);
        obj["type"] = endpoints.valueAt(i)->type;

        endpoints.valueAt(i)->fillValue(obj);
    }
}

void SpejsNode::httpMetadata(HttpRequest &request, HttpResponse &response)
{
    JsonObjectStream* stream = new JsonObjectStream();
    buildMetadata(stream);
    stream->getRoot()["uptime"] = millis();

    response.sendJsonObject(stream);
}

void SpejsNode::httpFile(HttpRequest &request, HttpResponse &response)
{
    String file = request.getPath();

    if (file[0] == '/')
        file = file.substring(1);

    if (file.startsWith("api/1/")) {
        String req = file.substring(6);
        String key = req.substring(0, req.indexOf("/"));
        String value = req.substring(req.indexOf("/") + 1);

        if(key.length() == 0 || value.length() == 0 || !endpoints.contains(key)) {
            response.badRequest();
        } else {
            EndpointResult result = endpoints[key]->onValue(key, value);
            JsonObjectStream* stream = new JsonObjectStream();
            JsonObject& json = stream->getRoot();
            json["status"] = result.status;
            response.sendJsonObject(stream);
        }
    } else if (file[0] == '.') {
        response.forbidden();
    } else {
        response.setCache(86400, true); // It's important to use cache for better performance.
        response.sendFile(file);
    }
}

void SpejsNode::onConnected() {
    Serial.println("Connection successful");

    // MQTT initialization
    mqtt.setWill(TOPIC_PREFIX + deviceID + "/state", "offline", 1, true);

#ifdef ENABLE_SSL
    const uint8_t sha1Fingerprint[] = SSL_FINGERPRINT;
    mqtt.connect("iot-" + deviceID, "", "", true);
    mqtt.addSslOptions(SSL_SERVER_VERIFY_LATER);
    mqtt.setSslFingerprint(sha1Fingerprint, 20);
#else
    mqtt.connect("iot-" + deviceID);
#endif

    for(unsigned int i = 0 ; i < endpoints.count() ; i++) {
        mqtt.subscribe(TOPIC_PREFIX + deviceID + "/" + endpoints.keyAt(i));
    }

    // Say hello
    mqtt.publish(TOPIC_PREFIX + deviceID + "/state", "online", true);

    JsonObjectStream stream;
    buildMetadata(&stream);
    char* metadataStr = new char[stream.getRoot().measureLength() + 10]();
    stream.getRoot().printTo(metadataStr, stream.getRoot().measureLength() + 10);
    mqtt.publish(TOPIC_PREFIX + deviceID + "/metadata", metadataStr, true);
    delete metadataStr;

    // HTTP initialization
    http.listen(80);

    http.addPath("/", HttpPathDelegate(&SpejsNode::httpIndex, this));
    http.addPath("/metadata.json", HttpPathDelegate(&SpejsNode::httpMetadata, this));
    http.setDefaultHandler(HttpPathDelegate(&SpejsNode::httpFile, this));

    // mDNS initialization
    static struct mdns_info *info = (struct mdns_info *)os_zalloc(sizeof(struct mdns_info));
    char tmp_name[32];
    ("iot-" + deviceID).toCharArray(tmp_name, 32);
    info->host_name = tmp_name; // You can replace test with your own host name
    info->ipAddr = WifiStation.getIP();
    info->server_name = (char *) "spejsiot";
    info->server_port = 80;

    char tmp_version[32] = "version=";
    int prefix_len = strlen(tmp_version);
    strncat(tmp_version, BUILD_ID, 32);
    info->txt_data[0] = tmp_version;

    char tmp_type[32];
    ("type=" + deviceType).toCharArray(tmp_type, 32);
    info->txt_data[1] = tmp_type;

    espconn_mdns_init(info);

    // Keepalive Timer initialization
    keepaliveTimer.initializeMs(10000, TimerDelegate(&SpejsNode::keepAliveHandler, this)).start();
}

bool SpejsNode::notify(String key, String value) {
    if(mqtt.getConnectionState() == eTCS_Connected) {
        mqtt.publish(TOPIC_PREFIX + deviceID + "/" + key + "/state", value, true);
        return true;
    } else {
        Serial.println("MQTT Not Connected!!!");
    }

    return false;
}

void SpejsNode::registerEndpoint(String key, Endpoint* endpoint) {
    endpoints[key] = endpoint;
    endpoint->bind(key, this);
}

void SpejsNode::mqttCallback(String origtopic, String value) {
    String devicePrefix = TOPIC_PREFIX + deviceID;

    if(!origtopic.startsWith(devicePrefix)) {
        Serial.println("ignoring");
        return;
    }

    String topic = origtopic.substring(devicePrefix.length() + 1);

    if(endpoints.contains(topic)) {
        endpoints[topic]->onValue(origtopic, value);
    } else {
        Serial.println("unknown topic?");
    }
}
