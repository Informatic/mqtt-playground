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

Mosquitto contained in docker is used as a broker. For proper TLS you need to
create self-signed certificate and store its SHA-1 fingerprint in
`common/common_config.h`.

ESP8266 code uses [slaff's
fork of Sming framework](https://github.com/slaff/Sming). (For axTLS support)
Set your Wifi configuration in `common/common_config.h` (used by both `switch`
and `light`)


Thoughts
--------

Oh my, that's slow.^W^Wquite fast, when patched properly.

**WARNING!** `Sming.reset();` jumps to (serial) bootloader right after flashing.
This causes OTA to fail with `wdt reset`. External RESET assert is required after
flashing.

TODO
----
 * Refactor endpoint handling
 * Fix mDNS
 * Store configuration (credentials, broker IP, maybe endpoints?) in
   flash memory
