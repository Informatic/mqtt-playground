#ifndef __COMMON_CONFIG_H__
#define __COMMON_CONFIG_H__

#ifndef WIFI_SSID
    #define WIFI_SSID "WIFI-NETWORK"
    #define WIFI_PWD "PASSWORD"
#endif

#ifndef MQTT_BROKER
    #define MQTT_BROKER "192.168.0.100"
    #ifdef ENABLE_SSL
        #define MQTT_PORT 8883
        #define SSL_FINGERPRINT { } // TODO
    #else
        #define MQTT_PORT 1883
    #endif
#endif

#define TOPIC_PREFIX "iot/"

#define OTA_URL "http://" MQTT_BROKER "/api/1/ota/"

#define BTN_PIN 0
#define LED_PIN 2

#endif
