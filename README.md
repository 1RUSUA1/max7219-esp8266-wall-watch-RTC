Version 2.5 with RTC and light level sensor 15-08-2020. Code blocks splitted to different files for reading usability.

This is my arduino code for wall-mounted watch, based on 4x max7219 8x8 LED dot matrix, ESP8266 module and RTC module on DS1302.

The watch takes time from RTC module and don't require NTP for the first launch. Every 24 hours by Ticker.h the time on RTC clock is being updated and corrected.
The watch has HTTP server. It accepts some configuration by HTTP request. All described inside /help page code.
Also there is automatic brightness correction, according to the current time. Values of Day and Night brightness are stored in EEPROM and could be set via HTTP request.
Automatic brightness correction could be done by external light level sensor, connected to ADC pin.This mode could be set via HTTP request.
Also there are includes OTA-update functionality, MDNS responder, SSDP server.
There is a function udpsend() for sending log strings to any Rsyslog server to UDP port 514.

The watch stores a Wifi connection data in EEPROM and read it while start,then trying to connect. If during 10 seconds its unable to connect, the watch starts working,
but a blinking second symbol ":" on LED changing to "!". Shorting the GPIO0 pin to GND the device could be activated as Wifi AP, where you can connect and type new Wifi 
access data on /wifi page. IP address is shown on LED after AP started.

External libs:
https://github.com/Makuna
https://github.com/markruys/arduino-Max72xxPanel
https://github.com/taranais/NTPClient
