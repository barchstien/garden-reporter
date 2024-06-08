//#include <ArduinoLowPower.h>

#include "battery.h"
#include "button.h"
#include "epoch_time_t.h"
#include "led.h"
#include "http_reporter.h"
#include "valve.h"
#include "wifi.h"

volatile uint32_t flow_cnt = 0;
volatile uint32_t flow_cnt_last = 0;

volatile uint32_t start_water_cnt = 0;
volatile uint32_t start_water_cnt_last = 0;

// Wifi report period
const unsigned int sec_report_period = (10 * 1); // TODO 15 min
// value returned by millis() when reported last
volatile unsigned long last_report_millis = 0;

// delay between loop
const unsigned int millisec_loop_step = (1000);

void flow_trig()
{
  flow_cnt ++;
}

//void start_water_trig()
//{
//  start_water_cnt ++;
//}

unsigned long b = 10UL;

battery_t battery;
button_t button;
epoch_time_t epoch_time;
led_t led;
valve_t valve;
wifi_t wifi;
http_reporter_t reporter;

// Command received by server
http_reporter_t::command_t server_cmd;

void setup()
{
  // debug
  Serial.begin(9600);
  Serial.println("--");
  Serial.println("-- setup");
  
  button.init();
  epoch_time.init();
  led.init();
  valve.init();

  // Suspecting low power sleep to make USB serial unstable !!!
  // |--> confirm later, use normal interrupt for now

  // blink
  led.blink(3, 1000, 50);
  
  // Why ?
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("--");
  Serial.println("-- setup END");

  // just to help debug
  delay(10000);

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
  // debug
  Serial.print("epoch time: ");
  Serial.println(epoch_time.sec_since_epoch());

  Serial.print("minutes: ");
  Serial.print(button.bit_value() * 15);
  Serial.print(" allowed: ");
  Serial.println(button.allow_water());

  Serial.print("Battery: ");
  Serial.println(battery.read_volt());
  // debug end

  // TODO delete
#if 0
  // debugu valve on/off
  digitalWrite(LED_BUILTIN, HIGH);
  valve.water_on();
  delay(10000);
  digitalWrite(LED_BUILTIN, LOW);
  valve.water_off();
  delay(20000);
#endif

  // HTTP report
  // Send status
  // Get config including epoch time sync and watering schedule
  if (last_report_millis == 0 ||
    epoch_time_t::sec_between_millis(last_report_millis, millis()) >= sec_report_period)
  {
    // Report via wifi
    wifi.connect();
    http_reporter_t::command_t cmd = reporter.report();
    if (cmd.is_valid)
    {
      Serial.println("=== CMD FROM SERVER ===");
      epoch_time.set_sec_since_epoch(cmd.sec_since_epoch);
      last_report_millis = millis();
      server_cmd = cmd;
    }
    wifi.end();
  }

  if (server_cmd.is_valid)
  {
    // A config has been received from server
    // TODO
    // check current epoch time vs next scheduled
    // add duration to current and set deadline
    // Keep in this scope until water should be off

    // TODO also check water allowed and battery min level
  }

  if (button.start_water_was_pushed())
  {
    // TODO also check battery min level
    if (button.allow_water())
    {
      Serial.println("Manual start !!!!");
    }
  }

  Serial.println("---------------");
  delay(millisec_loop_step);
}
