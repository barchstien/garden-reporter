#pragma once

#include "pin.h"

volatile bool start_water;

static void start_water_isr()
{
  start_water = true;
}

struct button_t
{
  void init()
  {
    // buttons
    pinMode(BIT0, INPUT_PULLUP);
    pinMode(BIT1, INPUT_PULLUP);
    pinMode(BIT2, INPUT_PULLUP);
    pinMode(ALLOW_WATER, INPUT_PULLUP);
    // to start watering of length bit_value() * 15min
    start_water = false;
    pinMode(START_TRIG, INPUT_PULLUP);
    attachInterrupt(
      digitalPinToInterrupt(START_TRIG), start_water_isr, FALLING
    );
  }

  // TODO read

  /** @return the value coded in 3 bit */
  int bit_value()
  {
    int b0 = digitalRead(BIT0);
    int b1 = digitalRead(BIT1);
    int b2 = digitalRead(BIT2);
    return b2 * 4 + b1 * 2 + b0; 
  }

  bool allow_water()
  {
    return digitalRead(ALLOW_WATER) == HIGH;
  }

  bool start_water_was_pushed()
  {
    if (start_water)
    {
      start_water = false;
      return true;
    }
    // else
    return false;
  }

  void debug_read()
  {
    Serial.print("debug button ");
    Serial.print(digitalRead(BIT0));
    Serial.print(" ");
    Serial.print(digitalRead(BIT1));
    Serial.print(" ");
    Serial.print(digitalRead(BIT2));
    Serial.print(" ");
    Serial.print(digitalRead(ALLOW_WATER));
    Serial.println("");
  }
};