# Water control
Control a solenoid valve, measure the outgoing water flow, report and take order using wifi + http GET

# HW
 * Arduino nano 33 iot (wifi)
   - [datasheet](../water-control/datasheet/arduino-nano-33-iot-datasheet.pdf)
   - [SAM-D21 (chip) datasheet](../water-control/datasheet/SAM-D21DA1-Family-Data-Sheet-DS40001882G.pdf)
 * Arduibox (DIN RAIL + power supply + PCB)
   - [Datasheet](../water-control/datasheet/Datasheet_ArduiBox_Rev_B-2.pdf)
   - [Construction manual](../water-control/datasheet/Construction_manual_ArduiBox_rev_B-4.pdf)
 * H bridge (N-mos)
   - commercial name : DRV8874 Single Brushed DC Motor Driver Carrier
   - [Official doc](https://www.pololu.com/product/4035)
   - [board schematic](../water-control/datasheet/drv887x-single-brushed-dc-motor-driver-carrier-schematic.pdf)
   - [drv8874 datasheet](../water-control/datasheet/drv8874.pdf)
 * DEPRECATED - hard to drive, blaming too low pressure
   Rainbird electro valve with 9V solenoid
   - 100-DVF
   - Rain Bird TBOS latching solenoid (does datasheet exists ?)
     Model ? TBOSPSOL or K80920  
 * Ball Valve 
   Normally closed, closes using a spring once power is off 
   - can control it using H-bridge meant for electrovalve
   - no need for battery anymore
   - TF20-B2-B 9-24VDC 
   - 3sec open/close motor stop when fully opened
 * Power supply
   - HDR-30-15
   - 15V 2A DIN rail
 * Adafruit DS3231 breakout board
   - [Official doc](https://www.adafruit.com/product/3013)
   - Note SAMD31 does have an RTC, but not crystal, so no good stability
 * Interface
   - LED
   - Start watering push button
   - 3 button to code 3 bits (x15min, to set watering duration)
   - On/Off button, forbid water use when off

## Schematics
TODO
ALSO check that rplacing 9V by 12V is fine !!!!!!!!!!!
Input power (now 9V) goes to :
 - arduibox voltage regulatr that takes 9 to 30 VDC
 - power rails of H-bridge takes 4.5 to 37V DC
|--> should be all fine !

## Power use
| Function  | Consumption mA  |
|--:|---|
|delay                    |25         |
|wifi connect             |45 to 85   |
|wifi low power mode      |45 to 55   |
|wifi end                 |25         |
|LowPower idle/sleep/deep |19
|coil driver no load      |2.5
|5V reg no load           |3.5
|flow meter alone         |2

# SW
using Arduino IDE, source is in [water-control](./water-control)  
Requires Arduino Libraires (to install with IDE) :  
 * ArduinoHttpClient (easy http GET)
 * ArduinoJson (Json http response)
 * Arduino_Low_Power (to confirm, to keep power usage to min)
 * RTCLib for Adafruit DS3231 breakout board
 * Time (unix to date/time, local time sync)
 * WiFiNINA (wifi connection + web client)

# Interface
```

|------1-2-3-4-5-6---7-8-9-10-11-12---|
|                                     |
|                                     |
|                                     |
|    Arduibox layout seen from top    |
|                                     |
|                                     |
|                                     |
|-----------------------------Gnd-15V-|
```
**Note:** [Pin #] as on drawing
| Pin # | Function |
|--:|---|
|1| batt sense |
|2| 3.3V |
|3| flow trig |
|4| gnd |
|5| LED (pwm) |
|6| start trig |
|||
|7| allow water |
|8| bit 2 |
|9| bit 1 |
|10| bit 0 |
|11| valve 0V/9V |
|12| valve 9V/0V |


## Button interface
```
|-1-2-3____4-5-6-7-|
|                  |
|                  |
|                  |
|                  |
|------------------|
```
**Note:** [Pin #] as on drawing
| Pin # | Function |
|--:|---|
|1| gnd |
|2| led |
|3| start trig |
|||
|4| allow water |
|5| bit 2 |
|6| bit 1 |
|7| bit 0 |

# TODO
 * Use RTC board square wave output at 1KHz, to get better msec precision for valve pulse ?
   |--> require to unmount from arduibox to solder new pin...
   |--> easy to setup 
   |--> only enable/use when doing valve pulse ?
        OR ? Could totally replace millis() ?
        With 48MHz M0 and 1024Hz sqw wave, it means 1 interrupt every 48K cycles... not bad
 * save in RTC flash (alarm), next epoch time and perdio/duration
   |--> if power comes back, and server isn't up <--- no so likely..
 * suspecting manual trigger to be canceled if can't wifi connect to server

# Notes
file:///home/bastien/while-true/common/garden-reporter/water-control/index.html?period=1&start-time=12%3A30&duration=66

# Bibliography
 * [Arduino-Nano-33-IoT-Ultimate-Guide](https://github.com/ostaquet/Arduino-Nano-33-IoT-Ultimate-Guide)
 * [Official](https://docs.arduino.cc/hardware/nano-33-iot)