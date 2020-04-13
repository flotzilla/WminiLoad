WminiLoad
=========

Arduino pc's monitoring tool

Description
-----------

This until gather PC's information (cpu, mem, video cards loading, fan stats) via serial port
and show it on LCD 4x20 screen

[Python project](https://github.com/flotzilla/ArdLoader) for gathering info from pc

Overview
---------
![stats_image](https://user-images.githubusercontent.com/3332506/78845554-89041400-7a11-11ea-8ca0-7d75aecd4009.jpg)

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
