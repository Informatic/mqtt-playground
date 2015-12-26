mqtt-playground
===============

This repository is a storage for my mqtt and IoT (( ͡° ͜ʖ ͡°)) playground.


Hardware
--------

Current tests involve NodeMCU ESP12-E/ESP8266 boards. Switch is activated with
GPIO0 (FLASH button on NodeMCU) and light is connected to GPIO2, where blue LED
is directly connected on ESP-12E module.


Software
--------

Mosquitto contained in docker is used as a broker. ESP8266 code uses [Sming
framework](https://github.com/SmingHub/Sming). Set your Wifi configuration in
`common/common_config.h` (used by both `switch` and `light`)


Thoughts
--------

Oh my, that's slow.
