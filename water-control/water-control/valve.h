#pragma once

#include "pin.h"

/**
 * From forums, latched solenoid valve need 20 to 50 msec puls
 * delay() is running @150%
 * 50 msec * 1.5 = 75 msec   1st fails
 * 40 msec * 1.5 = 60 msec   1st fails
 * 30 msec * 1.5 = 45 msec   No work
 * 20 msec * 1.5 = 30 msec   No work
 *                100 msec   1st fails, many fails after
 * 
 * Looks like 1st water fails after arduino is prog with USB ?
 * After 30+ tries (with different delays) :
 *  - 75 msec looks good
 *  - doesn't fails on 1st cycle anymore
 *  - no relation with USB prog
 * --> should wires we water protected ?
 * 
 * Few days later... impossible to open the valve
 * Two suspects : 
 *  1. corrosion on wires
 *  2. got to re-try open/close and log any problem
 * 
 * Wires are now twisted together and clamped
 * Been having an infinite loopn trying to close in vain !
 * --> Increase pulse duration ! Never had that before
 * Use 1000 msec, like before, was reliable
 * No much gain to have battery wise, because system would exhasut battery in few days only anyways
 * |--> Doesn't help ! Still can't close valve
 * |--> must be due to new wire twist + clamp
 * |--> Attach wires and solder, use
*/
#define VALVE_PULSE_MSEC 1000
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
    delay(250);
    water_off();
    // workaround to be safe
#if 0
    for (int i=0; i<3; i++)
    {
      delay(2000);
      water_off();
    }
#endif
    // TODO bring water off loop here
    // ... using pointer to flow_cnt !!!!!
    // TODO check if water is on using flow meter
    // ... and re water_on() if no flow, up to 10 times ?
    // 1st on/off cycle always fails... Why ?
    // Do an on/off now to work around
    //delay(1000);
    //water_on();
    //delay(3000);
    //water_off();
  }

  void water_on()
  {
    digitalWrite(VALVE_PHASE, VALVE_PHASE_OPEN);
    digitalWrite(VALVE_ENABLE, 1);
    delay(VALVE_PULSE_MSEC);
    digitalWrite(VALVE_ENABLE, 0);
    is_on_ = true;
  }

  void water_off()
  {
    digitalWrite(VALVE_PHASE, VALVE_PHASE_CLOSE);
    digitalWrite(VALVE_ENABLE, 1);
    delay(VALVE_PULSE_MSEC);
    digitalWrite(VALVE_ENABLE, 0);
    is_on_ = false;
  }

  bool is_on() const
  {
    return is_on_;
  }

private:
 bool is_on_{false};
};