#pragma once

#include "pin.h"

#define VALVE_PULSE_MSEC 500
#define VALVE_PHASE_OPEN 1
#define VALVE_PHASE_CLOSE 0

/**
* Controle DRV8874 Single Brushed DC Motor Driver Carrier
* Which is used as a driver for the single coil 9V latching relay
* mini board : https://www.pololu.com/product/4035
* It uses 2 PIN to control
* Enable | Phase | OUT1 | OUT2 |
* ------------------------------
*    0   |   X   |   L  |  L   |
* ------------------------------
*    1   |   1   |   H  |  L   |
* ------------------------------
*    1   |   0   |   L  |  H   |
* ------------------------------
*/
struct valve_t
{
  void init()
  {
    pinMode(VALVE_ENABLE, OUTPUT);
    pinMode(VALVE_PHASE, OUTPUT);
    water_off();
  }

  void water_on()
  {
    digitalWrite(VALVE_PHASE, VALVE_PHASE_OPEN);
    digitalWrite(VALVE_ENABLE, 1);
    delay(VALVE_PULSE_MSEC);
    digitalWrite(VALVE_ENABLE, 0);
  }

  void water_off()
  {
    digitalWrite(VALVE_PHASE, VALVE_PHASE_CLOSE);
    digitalWrite(VALVE_ENABLE, 1);
    delay(VALVE_PULSE_MSEC);
    digitalWrite(VALVE_ENABLE, 0);
  }
};