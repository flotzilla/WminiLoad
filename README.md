WminiLoad
=========

Arduino pc's monitoring tool

Description
-----------

This until gather PC's information (cpu, mem, video cards loading, fan stats) via serial port
and show it on LCD 16x4 screen

[Python project](https://github.com/flotzilla/WminiLoad) for gathering info from pc

Requirements
------------
* arduino / esp8266 / wemos board
* LCD 2004 screen with i2c module
* push button 
* 10kOm resiztor
* *optional* 10kOm variable resistor

Scheme
------------
1. Connect Lcd screen vith i2c module to arduino A1 -> SDA, A2->SCL / WEMOS-D1 D1 -> SDA, D2-> SCL
2. Connect push button to pin D5 on WEMOS / pin 14 on arduino 

*optional* add variable resistor i2c's lcd pin module