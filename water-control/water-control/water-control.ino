#include <ArduinoLowPower.h>

#include "button.h"
#include "led.h"
#include "http_client.h"
#include "valve.h"
#include "wifi.h"

volatile uint32_t flow_cnt = 0;
volatile bool start_water = false;

void flow_trig_isr()
{
  flow_cnt ++;
}

void start_water_isr()
{
  start_water = true;
}

button_t button;
http_client_t http_client;
led_t led;
valve_t valve;
wifi_t wifi;

void setup()
{
  // debug
  Serial.begin(9600);
  Serial.println("--");
  Serial.println("-- setup");
  
  button.init();
  led.init();
  valve.init();
  //wifi.init();

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

  // blink
  led.off();
  delay(1000);
  led.on();
  delay(1000);
  led.off();

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("--");
  Serial.println("-- setup END");
}

void loop()
{
#if 0
  // debugu valve on/off
  digitalWrite(LED_BUILTIN, HIGH);
  valve.water_on();
  delay(10000);
  digitalWrite(LED_BUILTIN, LOW);
  valve.water_off();
  delay(20000);
#endif

#if 0
  // debug wifi and http
  Serial.println("-- Loop");
  Serial.println("-- wifi waky");
  wifi.wakeup();
  wifi.debug();
  Serial.println("-- http GET");
  http_client.report();
  wifi.sleep();

  delay(10000);
#endif

#if 0
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
#endif
}
