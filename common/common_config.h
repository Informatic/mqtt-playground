#ifndef __COMMON_CONFIG_H__
#define __COMMON_CONFIG_H__

#ifndef WIFI_SSID
    #define WIFI_SSID "WIFI-NETWORK"
    #define WIFI_PWD "PASSWORD"
#endif

#ifndef MQTT_BROKER
    #define MQTT_BROKER "192.168.0.100"
    #define MQTT_PORT 1883
#endif

#define BTN_PIN 0
#define LED_PIN 2

#endif
