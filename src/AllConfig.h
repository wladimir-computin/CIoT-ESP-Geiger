/*
* CryptoGarage - AllConfig
* Compiletime constants and defines.
* 
* Arduino IDE processes files in alphabetical order, hence the defines MUST be in a file named beginning with an "A"...
*/

#ifndef ALLCONFIG
#define ALLCONFIG
  const int I2C_DISPLAY_ADDRESS = 0x3c;
  #ifdef ARDUINO_ARCH_ESP32
    #define SDA_PIN 5
    #define SDC_PIN 4
    #define INT_PIN 13
  #else
    #define SDA_PIN D1
    #define SDC_PIN D2
    #define INT_PIN D7
  #endif

  const double SBM_20_CONV_RATE = 175;

  const char DEVICETYPE[] = "CryptoGeiger";

#endif
