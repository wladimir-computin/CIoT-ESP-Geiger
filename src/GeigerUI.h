/*
  GeigerUI

  Class for visualizing Geiger Data
*/

#pragma once

#include <Arduino.h>

#ifdef ARDUINO_ARCH_ESP32
  #include <Wire.h>
  #include <SSD1306Wire.h>
#else
  #include <brzo_i2c.h>
  #include <SH1106Brzo.h>
#endif

#include "AllConfig.h"
#include <App.h>
#include <PrintDebug.h>
#include <PersistentMemory.h>

#include "Geiger.h"
#include "Helper.h"

enum GuiModes {DISPLAY_OFF, NORMAL, FAST, PRECISION, PARTICLE, BAND};

static Param COMMAND_GETGEIGER_TICKARRAY_PARAMS[] = {{"value", DATATYPE_STRING, true}};
static Command COMMAND_GETGEIGER_TICKARRAY("getgeigerarr", ARRAY_LEN(COMMAND_GETGEIGER_TICKARRAY_PARAMS), COMMAND_GETGEIGER_TICKARRAY_PARAMS);

static Param COMMAND_GUIMODE_PARAMS[] = {{"value", DATATYPE_STRING, true}};
static Command COMMAND_GUIMODE("guimode", ARRAY_LEN(COMMAND_GUIMODE_PARAMS), COMMAND_GUIMODE_PARAMS);

static Command COMMAND_GETGEIGER("getgeiger");

const char GUIMODE_OFF[] = "off";
const char GUIMODE_NOMRAL[] = "normal";
const char GUIMODE_PRECISION[] = "precision";
const char GUIMODE_FAST[] = "fast";
const char GUIMODE_PARTICLE[] = "particle";
const char GUIMODE_BAND[] = "band";

class GeigerUI : public App{

  public:
    GeigerUI(const char * name, Geiger &geiger);
    ProcessMessageStruct processMessage(String &message);
    String getName();
    String getStatus();
	String getType();
    void setup();
    void loop();

  private:
    Geiger &geiger;
    
    int FPS = 60;

    long cpm = 0;
    double uSv_h = 0;

    long cpm_show = 0;
    long cpm_show_old = 0;
    double cpm_show_rise = 0;
    int cpm_show_x = 0;

    double uSv_h_show = 0;
    double uSv_h_show_old = 0;
    double uSv_h_show_rise = 0;
    int uSv_h_show_x = 0;

    int time_passed = 0;
    int brightness = 1;
    int splash = 0;
    unsigned long rendermicros = 0;
    GuiModes guiMode = NORMAL;

    void render(bool clear);
    void renderBand(bool clear);
    void renderParticles(bool clear);
    void setGuiMode(GuiModes mode);
    String mode2string(GuiModes m);
    GuiModes string2mode(String m);
};
