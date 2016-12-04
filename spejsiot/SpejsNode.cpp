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

    registerEndpoint("$implementation", new ImplementationEndpoint());
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
                //Serial.printf("Not a TEMP ROM boot: %d\r\n", mode);
            }
        } else {
            //Serial.println("No boot mode info");
        }
    }
}

#define DEV_TOPIC(t) (TOPIC_PREFIX + deviceID + "/" + t)

void SpejsNode::httpIndex(HttpRequest &request, HttpResponse &response)
{
    response.sendString("This is spejsiot device, take a look at: https://wiki.hackerspace.pl/projects:spejsiot\n"
        "\nDevice type: " + deviceType +
        "\nFirmware version: " + String(BUILD_ID) +
        "\nMAC: " + WifiStation.getMAC());
}

void SpejsNode::onConnected() {
    Serial.println("Connection successful");

    // MQTT initialization
    mqtt.setWill(TOPIC_PREFIX + deviceID + "/$online", "false", 1, true);

#ifdef ENABLE_SSL
    const uint8_t sha1Fingerprint[] = SSL_FINGERPRINT;
    mqtt.connect("iot-" + deviceID, "", "", true);
    mqtt.addSslOptions(SSL_SERVER_VERIFY_LATER);
    mqtt.setSslFingerprint(sha1Fingerprint, 20);
#else
    mqtt.connect("iot-" + deviceID);
#endif

    for(unsigned int i = 0 ; i < endpoints.count() ; i++) {
        mqtt.subscribe(TOPIC_PREFIX + deviceID + "/" + endpoints.keyAt(i) + "/+/set");
    }
    mqtt.subscribe(TOPIC_PREFIX + deviceID + "/$implementation/+");

    // Say hello
    mqtt.publish(DEV_TOPIC("$online"), "true", true);
    mqtt.publish(DEV_TOPIC("$homie"), "2", true);
    mqtt.publish(DEV_TOPIC("$name"), deviceType, true);
    mqtt.publish(DEV_TOPIC("$localip"), WifiStation.getIP().toString(), true);
    mqtt.publish(DEV_TOPIC("$mac"), WifiStation.getMAC(), true);
    mqtt.publish(DEV_TOPIC("$fw/name"), "spejsiot", true);
    mqtt.publish(DEV_TOPIC("$fw/version"), BUILD_ID, true);

    for(unsigned int i = 0; i < endpoints.count(); i++) {
        mqtt.publish(DEV_TOPIC(endpoints.keyAt(i) + "/$type"), endpoints.valueAt(i)->type, true);
    }

    // HTTP initialization
    http.listen(80);
    http.addPath("/", HttpPathDelegate(&SpejsNode::httpIndex, this));
    http.setDefaultHandler(HttpPathDelegate(&SpejsNode::httpFile, this));

    // mDNS initialization
    initializeMDNS();

    // Keepalive Timer initialization
    keepaliveTimer.initializeMs(10000, TimerDelegate(&SpejsNode::keepAliveHandler, this)).start();
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
    }
}


void SpejsNode::initializeMDNS() {
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
}

bool SpejsNode::notify(String key, String value) {
    if(mqtt.getConnectionState() == eTCS_Connected) {
        mqtt.publish(TOPIC_PREFIX + deviceID + "/" + key, value, true);
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

    int propPos = origtopic.indexOf("/", devicePrefix.length() + 1);
    String endpoint = origtopic.substring(devicePrefix.length() + 1, propPos);
    String property = origtopic.substring(propPos+1, origtopic.indexOf("/", propPos+1));

    if(endpoints.contains(endpoint)) {
        Serial.printf("%s - %s response: %d\n", endpoint.c_str(), property.c_str(), endpoints[endpoint]->onValue(property, value).status);
    } else {
        Serial.println("unknown topic? " + endpoint);
    }
}
