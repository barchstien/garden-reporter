#include <ArduinoLowPower.h>
#include <WiFiNINA.h>

#include "arduino_secrets.h"
#include "pin.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

volatile uint32_t flow_cnt = 0;
volatile bool start_water = false;

struct wifi_t
{
  wifi_t()
    : is_connected(false)
  {}

  void wakeup()
  {
    WiFi.setTimeout(30000);
    int status = WL_IDLE_STATUS;
    status = WiFi.begin(ssid, pass);
    is_connected = status == WL_CONNECTED;
  }

  void sleep()
  {
    WiFi.end();
  }

  bool is_connected;
};

void flow_trig_isr()
{
  flow_cnt ++;
}

void start_water_isr()
{
  start_water = true;
}

void water_on()
{
  digitalWrite(VALVE_OPEN, 1);
  digitalWrite(VALVE_ENABLE, 1);
  delay(500);
  digitalWrite(VALVE_ENABLE, 0);
}

void water_off()
{
  digitalWrite(VALVE_OPEN, 0);
  digitalWrite(VALVE_ENABLE, 1);
  delay(500);
  digitalWrite(VALVE_ENABLE, 0);
}

wifi_t wifi;

void setup()
{
  // water
  pinMode(VALVE_ENABLE, OUTPUT);
  pinMode(VALVE_OPEN, OUTPUT);
  water_off();
  // led
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 1);
  // buttons
  pinMode(BIT0, INPUT_PULLUP);
  pinMode(BIT1, INPUT_PULLUP);
  pinMode(BIT2, INPUT_PULLUP);
  pinMode(ALLOW_WATER, INPUT_PULLUP);
  // trigs
  attachInterrupt(
    digitalPinToInterrupt(FLOW_TRIG), flow_trig_isr, RISING
  );
  attachInterrupt(
    digitalPinToInterrupt(START_TRIG), start_water_isr, FALLING
  );
  LowPower.attachInterruptWakeup(
    digitalPinToInterrupt(FLOW_TRIG), flow_trig_isr, RISING
  );
  LowPower.attachInterruptWakeup(
    digitalPinToInterrupt(START_TRIG), start_water_isr, FALLING
  );
  // wifi
  wifi.wakeup();
  // led off
  digitalWrite(LED, 0);
  wifi.sleep();
}

void loop()
{
  if (start_water)
  {
    start_water = false;
    // TODO read button, start water
  }
  else
  {
    // woke up with timer
    // TODO http to /report
    wifi.wakeup();
  }

  LowPower.sleep(10000);
}
