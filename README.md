# CIoT-ESP-Geiger
CryptoIoT / Geiger Counter with OLED UI

## Build

```bash
git clone https://github.com/wladimir-computin/CIoT-ESP-Geiger.git
git clone https://github.com/wladimir-computin/CryptoIoT-Core.git
cd CIoT-ESP-Geiger
make
# for ESP32
make upload build=nodemcu-32s
# for ESP8266
make upload build=d1_mini_more_ram
```

## Related Projects

* [CryptoIoT-Core](https://github.com/wladimir-computin/CryptoIoT-Core.git)
	- The main core of the CryptoIoT smarthome system for ESP8266/ESP32
	- Handles WiFi, communication, encryption, persisten data storage etc.
* [CryptoIoT-Python](https://github.com/wladimir-computin/CryptoIoT-Python.git)
	- CLI client for interfacing with CryptoIoT devices.
* [Android-CryptoHouse](https://github.com/wladimir-computin/Android-CryptoHouse.git)
	- Natively written Android-App for controlling all CryptoIoT devices
