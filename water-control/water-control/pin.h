#pragma once

// from https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
// interrupt pins : 2, 3, 9, 10, 11, 13, A1, A5, A7

#define VALVE_ENABLE    A2
#define VALVE_PHASE     A3
#define FLOW_TRIG       9
#define LED             3   // PWM
#define BIT0            4
#define BIT1            5
#define BIT2            6
#define START_TRIG      10
#define ALLOW_WATER     7
#define BATT_READ       A0

// just for reference, ie not used in code
// i2c used for RTC
#define I2C_SDA         A4
#define I2C_SCL         A5