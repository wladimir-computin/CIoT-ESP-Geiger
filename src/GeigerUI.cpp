/*
  GeigerUI

  (implementation)
*/

#include "GeigerUI.h"


#ifdef ARDUINO_ARCH_ESP32
  SSD1306Wire display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN, GEOMETRY_128_64, I2C_ONE, 1000000);
#else
  SH1106Brzo display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
#endif

GeigerUI::GeigerUI(const char * name, Geiger &_geiger) : geiger(_geiger) {
  registerCommand(COMMAND_GUIMODE);
  registerCommand(COMMAND_GETGEIGER);
  registerCommand(COMMAND_GETGEIGER_TICKARRAY);
}

ProcessMessageStruct GeigerUI::processMessage(String &message) {

  if (COMMAND_GETGEIGER.check(message)) {
    JsonDocument json;
    long cpm_fast = geiger.getCPM_fast();
    long cpm_precise = geiger.getCPM_accurate();
    json["cpm_fast"] = cpm_fast;
    json["cpm_accurate"] = cpm_precise;
    json["uSv_h_fast"] = geiger.to_uSvh(cpm_fast);
    json["uSv_h_accurate"] = geiger.to_uSvh(cpm_precise);
    String out;
    serializeJsonPretty(json, out);
    return {DATA, out};
  }

  if (COMMAND_GETGEIGER_TICKARRAY.check(message)) {
    int index = geiger.getClickIndex();
    String out = String("{\"index\":") + index;
    out += ",\"ticks\":[";
    int x = 0;
    for (int i = index; x < MEASURES_PER_MINUTE; i = mod(i - 1, MEASURES_PER_MINUTE)) {
      out += String(geiger.getClickArr()[i]) + ",";
      x++;
    }
    out += "]}";
    return {DATA, out};
  }

  if (COMMAND_GUIMODE.check(message)) {
    String param = COMMAND_GUIMODE.getParamVal(message, 0);
    if (param == "") {
      return {DATA, mode2string(guiMode)};
    } else {
      guiMode = string2mode(param);
    }
    return {ACK, ""};
  }

  return App::processMessage(message);

}

String GeigerUI::getName() {
  return geiger.getName();
}

String GeigerUI::getStatus() {
  return String("CPM: ") + cpm + "\n" +
         String("Dosis: ") + formatString(uSv_h, "%01.3f") + " µSv/h";
}

String GeigerUI::getType() {
  return "GEIGER_COUNTER_UI";
}


long previous_counts = 0;

struct Band {
  double a = 0;
  int t_int = 0;
};

Band band;

void GeigerUI::renderBand(bool clear) {
  long current_counts = geiger.getTotalTicks();
  long new_counts = abs(current_counts - previous_counts);
  previous_counts = current_counts;

  double usv = geiger.to_uSvh(geiger.getCPM_fast());

  band.a += 0.007 * ((double)new_counts / 2.0);
  band.a = min(band.a, 0.009);

  if (clear) {
    display.clear();
  }
  for (int x = -64; x < 64; x++) {
    display.setPixel(x + 64, round((band.a * sin(((double)band.t_int * 8 * PI * 2) / FPS) * cos(0.1 * usv * x) * (x + 64) * (x - 64)) + 32));
  }
  display.display();

  if (band.a > 0.00001) {
    band.a *= 0.85;
  } else {
    band.a = 0;
  }

  band.t_int += 1;
  band.t_int %= FPS;
}

struct Particle {
  int start_x = 0;
  int start_y = 0;
  int x = start_x;
  int y = start_y;
  int end_x = 0;
  int end_y = 0;
  double speed = 1;
  double way = 0;
  bool active = false;
};

Particle particles[100];

void GeigerUI::renderParticles(bool clear) {
  long current_counts = geiger.getTotalTicks();
  long new_counts = abs(current_counts - previous_counts);
  previous_counts = current_counts;

  for (int i = 0; i < new_counts; i++) {
    for (int i = 0; i < (sizeof(particles) / sizeof(particles[0])); i++) {
      Particle &p = particles[i];
      if (!p.active) {
        p.start_x = random(0, 128);
        p.start_y = 0;
        p.end_x = random(-32, 164);
        p.end_y = random(63, 128);
        p.x = p.start_x;
        p.y = p.start_y;
        p.way = 0;
        p.speed = 0.1 * (30.0 / FPS);
        p.active = true;
        break;
      }
    }
  }

  for (int i = 0; i < (sizeof(particles) / sizeof(particles[0])); i++) {
    Particle &p = particles[i];
    if (p.x < 0 || p.x > 128 || p.y < 0 || p.y > 64) {
      p.active = false;
    } else if (p.way >= 1) {
      p.active = false;
    } else if (p.active) {
      int vx = p.end_x - p.start_x;
      int vy = p.end_y - p.start_y;
      p.x = round(p.start_x + (double)vx * p.way);
      p.y = round(p.start_y + (double)vy * p.way);
      p.way += p.speed;
      p.speed *= 0.98;
    }
  }


  display.clear();
  for (int i = 0; i < (sizeof(particles) / sizeof(particles[0])); i++) {
    Particle &p = particles[i];
    if (p.active) {
      display.setPixel(p.x, p.y);
      if (p.y >= 63) {
        display.setPixel(p.x - 3, 63);
        display.setPixel(p.x - 1, 63);
        display.setPixel(p.x, 63);
        display.setPixel(p.x + 1, 63);
        display.setPixel(p.x + 3, 63);
      }
    }
  }
  display.display();
}

void GeigerUI::render(bool clear) {

  if (guiMode == NORMAL || guiMode == FAST || guiMode == PRECISION) {
    if (clear) {
      display.clear();
    }
    long cpm_new;

    if (guiMode != FAST || time_passed >= 1 * FPS) {
      if (guiMode == FAST) {
        cpm_new = geiger.getCPM_fast();
      } else {
        cpm_new = geiger.getCPM_accurate();
      }

      if (cpm_new != cpm) {
        if (guiMode == FAST && abs(cpm_new - cpm) > 1) {
          cpm = cpm_new;
          cpm_show_rise = (double)(cpm - cpm_show) / (FPS / 2);
          cpm_show_old = cpm_show;
          cpm_show_x = 0;
          //clear = true;
        } else {
          cpm = cpm_new;
        }
      }
    }

    if (time_passed >= 1 * FPS) {
      if (guiMode != PRECISION) {
        cpm_new = geiger.getCPM_fast();
      } else {
        cpm_new = geiger.getCPM_accurate();
      }

      time_passed = 0;
      uSv_h = geiger.to_uSvh(cpm_new);
      uSv_h_show_rise = (uSv_h - uSv_h_show) / (FPS / 2);
      uSv_h_show_old = uSv_h_show;
      uSv_h_show_x = 0;
      //clear = true;
    } else {
      time_passed++;
    }

    if (cpm_show_x < (FPS / 2)) {
      cpm_show = round(cpm_show_rise * cpm_show_x++) + cpm_show_old;
    } else if (cpm_show_x >= (FPS / 2)) {
      cpm_show = cpm;
    }

    if (uSv_h_show_x < (FPS / 2)) {
      uSv_h_show = uSv_h_show_rise * uSv_h_show_x++ + uSv_h_show_old;
    } else if (uSv_h_show_x >= (FPS / 2)) {
      uSv_h_show = uSv_h;
    }


    String sievert_number;
    String sievert_unit;
    const char formatstr[][7] = {"%01.3f", "%01.3f", "%01.2f"};

    if (uSv_h_show < 1000) {
      sievert_number = formatString(uSv_h_show, formatstr[count_digit(uSv_h_show) - 1]);
      sievert_unit = "µSv/h";
    } else {
      double mSv_h_show = uSv_h_show / 1000;
      if (mSv_h_show < 1000) {
        sievert_number = formatString(mSv_h_show, formatstr[count_digit(mSv_h_show) - 1]);
        sievert_unit = "mSv/h";
      } else { // not gonna happen (I hope)
        double Sv_h_show = mSv_h_show / 1000;
        sievert_number = formatString(Sv_h_show, formatstr[count_digit(Sv_h_show) - 1]);
        sievert_unit = " Sv/h";
      }
    }

    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(128, 7, "CPM");
    display.setFont(ArialMT_Plain_24);
    display.drawString(85, 0, String(cpm_show));

    display.setFont(ArialMT_Plain_16);
    display.drawString(128, 35, sievert_unit);
    display.setFont(ArialMT_Plain_24);
    display.drawString(80, 28, sievert_number);
    // write the buffer to the display

    int x = 0;
    double multiplier = 2;
    if (uSv_h_show > 3) {
      multiplier = 3 / uSv_h_show;
    }
    for (int i = geiger.getClickIndex(); x < 128; i = mod(i - 1, MEASURES_PER_MINUTE)) {
      int counts = geiger.getClickArr()[i];
      counts = round((double)counts * multiplier);
      for (; counts > 0; counts--) {
        display.setPixel(x, 64 - counts);
      }
      x++;
    }

    display.display();
  } else {
    if (guiMode == PARTICLE) {
      renderParticles(true);
    } else if (guiMode == BAND) {
      renderBand(true);
    } else {
      display.clear();
      display.display();
    }
  }
}

String GeigerUI::mode2string(GuiModes m) {
  switch (m) {
    case DISPLAY_OFF:
      return "OFF";
    case NORMAL:
      return "NORMAL";
    case FAST:
      return "FAST";
    case PRECISION:
      return "PRECISION";
    case PARTICLE:
      return "PARTICLE";
    case BAND:
      return "BAND";
  }
  return "NORMAL";
}

GuiModes GeigerUI::string2mode(String m) {
  m.toUpperCase();
  if (m == "OFF") {
    return DISPLAY_OFF;
  }
  if (m == "NORMAL") {
    return NORMAL;
  }
  if (m == "FAST") {
    return FAST;
  }
  if (m == "PRECISION") {
    return PRECISION;
  }
  if (m == "PARTICLE") {
    return PARTICLE;
  }
  if (m == "BAND") {
    return BAND;
  }
  return NORMAL;
}

void GeigerUI::setup() {
  geiger.start();

  display.init();
  display.drawXbm(32, 0, radioactive_sign_width, radioactive_sign_height, radioactive_sign_bits);
  display.display();
  rendermicros = micros() + (1000 * 1000 * 1);
}

void GeigerUI::loop() {
  if ((long)(micros() - rendermicros) >= ((1000 * 1000) / FPS)) {
    if (splash == -1) {
      render(true);

    } else {
      switch (splash) {
        case 0:
          {
            splash = 1;
          }
          break;

        case 1:
          {
            if (brightness < 75) {
              brightness *= 2;
              display.clear();
              display.fillCircle(64, 32, brightness);
              display.display();
            } else {
              splash = 2;
            }
          }
          break;

        case 2:
          {
            if (brightness > 0) {
              brightness /= 2;
              display.clear();
              display.fillCircle(64, 32, brightness);
              display.display();
            } else {
              splash = -1;
            }
          }
          break;
      }
    }
    rendermicros = micros();
  }
}
