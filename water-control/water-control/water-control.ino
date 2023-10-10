#include <ArduinoLowPower.h>

#include "battery.h"
#include "button.h"
#include "led.h"
#include "http_client.h"
#include "valve.h"
#include "wifi.h"

volatile uint32_t flow_cnt = 0;
volatile uint32_t flow_cnt_last = 0;

volatile uint32_t start_water_cnt = 0;

void flow_trig_isr()
{
  flow_cnt ++;
}

void start_water_trig()
{
  start_water_cnt ++;
}

unsigned long b = 10UL;

battery_t battery;
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
  // init() waits for NTP sync, then disconnect
  //wifi.init();
  //wifi.end();

// TODO delete !!!
#if 0
  // trigs
  attachInterrupt(
    digitalPinToInterrupt(START_TRIG), start_water_isr, FALLING
  );
  LowPower.attachInterruptWakeup(
    digitalPinToInterrupt(FLOW_TRIG), flow_trig_isr, RISING
  );
  LowPower.attachInterruptWakeup(
    digitalPinToInterrupt(START_TRIG), start_water_isr, FALLING
  );
#endif
  // trigs
  attachInterrupt(
    digitalPinToInterrupt(START_TRIG), start_water_trig, RISING
  );
  attachInterrupt(
    digitalPinToInterrupt(FLOW_TRIG), flow_trig_isr, RISING
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
#if 0
  // debug
  delay(30000);
  valve.water_on();
  delay(60000);
  valve.water_off();
#endif
}

void loop()
{
#if 0
  delay(10000);
  http_client.report();
#endif

#if 0
  Serial.println("End wifi, then re-connect in 10 sec");
  wifi.end();
  delay(10000);
  Serial.println("re-connecting...");
  wifi.init();
#endif

#if 1
  button.debug_read();
  Serial.print("start_water_cnt: ");
  Serial.println(start_water_cnt);
  Serial.print("battery voltage: ");
  Serial.println(battery.read_volt());
  
  delay(200);
#endif

#if 0
  delay(5000);
  Serial.println("-- delay for 10 sec");
  delay(10000);
  Serial.println("-- wifi init 10 sec");
  wifi.init();
  delay(10000);
  Serial.println("-- wifi sleep 10 sec");
  wifi.sleep();
  delay(10000);
  Serial.println("-- wifi end10 sec");
  wifi.end();
  delay(10000);
#endif

#if 0
  delay(5000);
  Serial.println("-- delay for 10 sec");
  delay(10000);
  Serial.println("-- ");
  LowPower.idle(10000);
  Serial.println("-- ");
  LowPower.sleep(10000);
  Serial.println("-- ");
  LowPower.deepSleep(10000);
#endif

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
  wifi.connect();
  Serial.println("-- http GET");
  http_client.report();
  wifi.end();

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
