/*
  Geiger

  (implementation)
*/

#include "Geiger.h"

volatile static int clickIndex = 0;
static uint16_t clickArr[MEASURES_PER_MINUTE];
volatile static long totalTicks = 0;

Geiger::Geiger(const char * name, int pin, double cpm_Sv_rate) {
  appname = name;
  this->pin = pin;
  this->cpm_Sv_rate = cpm_Sv_rate;
}

void Geiger::setup() {
  PersistentMemory pmem(appname, true);
  pin = pmem.readInt(KEY_PIN, pin);
  cpm_Sv_rate = pmem.readDouble(KEY_SV_RATE, cpm_Sv_rate);
  pmem.commit();

  pinMode(pin, INPUT);
}

String Geiger::getStatus() {
  return "";
}

String Geiger::getType() {
  return "GEIGER_COUNTER";
}


ProcessMessageStruct Geiger::processMessage(String &message) {
  return App::processMessage(message);
}

void Geiger::start() {
  for (int i = 0; i < MEASURES_PER_MINUTE; i++) {
    clickArr[i] = 0;
  }
  attachInterrupt(pin, geigerClick, RISING);
  geigerTicker.attach_ms(1000 / MEASURES_PER_SECOND, geigerTick, (void*)this);
}

void Geiger::stop() {
  detachInterrupt(digitalPinToInterrupt(pin));
  geigerTicker.detach();
}

void IRAM_ATTR Geiger::geigerClick() {
  clickArr[clickIndex]++;
  totalTicks++;
}

void IRAM_ATTR Geiger::geigerTick(void * context) {
  int ci = clickIndex;
  ci =  mod(ci + 1, MEASURES_PER_MINUTE);
  clickArr[ci] = 0;
  clickIndex = ci;
}

long Geiger::getCPM_accurate() {
  long cpm_slow = 0;
  for (int i = 0; i < MEASURES_PER_MINUTE; i++) {
    cpm_slow += clickArr[i];
  }
  return cpm_slow;
}

long Geiger::getCPM_fast() {
  int _clickIndex = clickIndex;
  int timeslots = 0;
  long cpm_fast = 0;

  while ((cpm_fast <= 10 || timeslots < 5 * MEASURES_PER_SECOND) && (timeslots < MEASURES_PER_MINUTE)) {
    cpm_fast += clickArr[mod(_clickIndex--, MEASURES_PER_MINUTE)];
    timeslots++;
  }
  cpm_fast = round((double)cpm_fast * (double)MEASURES_PER_MINUTE / (double)timeslots);

  return cpm_fast;
}

double Geiger::to_uSvh(long cpm) {
  double uSv_h = (double)cpm / cpm_Sv_rate;
  return uSv_h;
}

uint16_t * Geiger::getClickArr() {
  return clickArr;
}

int Geiger::getClickIndex() {
  return clickIndex;
}

long Geiger::getTotalTicks() {
  return totalTicks;
}

void Geiger::loop() {
  /*if ((micros() - measuremicros) >= 1000*1000 / MEASURES_PER_SECOND) {
    clickIndex = mod(clickIndex + 1, MEASURES_PER_MINUTE);
    clickArr[clickIndex] = 0;
    measuremicros = micros();
    }
  */
}

String Geiger::getName() {
  return appname;
}
