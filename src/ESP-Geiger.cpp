/*
  CryptoGeiger - Main

  Contains the main functionality.
  Entrypoint is setup()
*/

#include "AllConfig.h"
#include <CryptoIoT.h>
#include "Geiger.h"
#include "GeigerUI.h"

//Apps
Geiger geiger("Geiger", INT_PIN, SBM_20_CONV_RATE);
GeigerUI geigerUI("Geiger", geiger);
App * apps[] = {&geiger, &geigerUI};

CryptoIoT cryptoIoT(apps, sizeof(apps) / sizeof(apps[0]));

void loop() {
  cryptoIoT.loop();
}

//Here's the entrypoint.
void setup() {
  cryptoIoT.setup();
  cryptoIoT.setDeviceType(DEVICETYPE);
}
