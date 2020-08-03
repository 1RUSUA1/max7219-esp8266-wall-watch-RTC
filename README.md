Version 2.0 with RTC 02-08-2020


This is my arduino code for wall-mounted watch, based on 4x max7219 8x8 LED dot matrix, ESP8266 module and RTC module on DS1302.


The watch takes time from RTC module and don't require NTP for the first launch. Every 24 hours by Ticker.h the time on RTC clock is being updated and corrected.


The watch has HTTP server. It accepts some configuration by HTTP request. All described inside help.html page code.


Also there is automatic brightness correction, according to the current time. Values of Day and Night brightness are stored in EEPROM and could be set via HTTP request.


Also there are includes OTA-update functionality, MDNS responder, SSDP server.


There is a function udpsend() for sending log strings to any Rsyslog server to UDP port 514.
