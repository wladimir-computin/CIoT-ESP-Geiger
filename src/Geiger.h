/*
  Geiger
  
  Class for controlling RH Electronics DIY Geigercounter
*/

#pragma once

#include <Arduino.h>
#include <Ticker.h>
#include <ArduinoJson.h>

#include "AllConfig.h"
#include <App.h>
#include <PersistentMemory.h>

#include "Helper.h"

const char KEY_PIN[] = "tube_pin";
const char KEY_SV_RATE[] = "tube_rate";

const int MEASURES_PER_SECOND = 4;
const int MEASURES_PER_MINUTE = MEASURES_PER_SECOND * 60;

class Geiger : public App{
  
  public:

    Geiger(const char * name, int pin, double cpm_Sv_rate);
    ProcessMessageStruct processMessage(String &message);
    void setup();
    void loop();
    String getName();
    String getStatus();
	String getType();
	
    long getCPM_accurate();
    long getCPM_fast();
    double to_uSvh(long cpm);
    uint16_t * getClickArr();
    int getClickIndex();
    long getTotalTicks();
    
    void start();
    void stop();

  private:
    String appname;
    double cpm_Sv_rate;
    int pin;
    
    long measuremicros = 0;

    static void geigerClick();
    Ticker geigerTicker;
    static void geigerTick(void * context);
};
