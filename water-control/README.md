# Water control
Control a solenoid valve, measure the outgoing water flow, report and take order using wifi + http GET

# HW
 * Arduino nano 33 iot (wifi)
   - [datasheet](../water-control/datasheet/arduino-nano-33-iot-datasheet.pdf)
   - [SAM-D21 (chip) datasheet](../water-control/datasheet/SAM-D21DA1-Family-Data-Sheet-DS40001882G.pdf)
 * Arduibox (DIN RAIL + power supply + PCB)
   - [Datasheet](../water-control/datasheet/Datasheet_ArduiBox_Rev_B-2.pdf)
   - [Construction manual](../water-control/datasheet/Construction_manual_ArduiBox_rev_B-4.pdf)
 * Solenoid single coil driver (N-mos H bridge ++)
   - commercial name : DRV8874 Single Brushed DC Motor Driver Carrier
   - [Official doc](https://www.pololu.com/product/4035)
   - [board schematic](../water-control/datasheet/drv887x-single-brushed-dc-motor-driver-carrier-schematic.pdf)
   - [drv8874 datasheet](../water-control/datasheet/drv8874.pdf)
 * UPS 9v:
   - Provide uninterrupted 9V, use lithium batteries, power from USB-C
   - commercial name : Type-C 15W 3A 18650 Lithium Battery Charger Module DC-DC Step Up Booster Fast Charge UPS Power Supply / Converter 9V
   - Any 9V UPS with equivalent current is fine too
   - got it from aliexpress
 * Interface
   - LED
   - Start watering push button
   - 3 button to code 3 bits (x15min, to set wtaering duration)
   - On/Off button, forbid water use when off

## Schematics
TODO

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
Requires Arduino Libraires (to on install with IDE) :  
 * ArduinoHttpClient (easy http GET)
 * ArduinoJson (Json http response)
 * Arduino_Low_Power (to confirm, to keep power usage to min)
 * RTCZero (No RTC module installed, would be a great addon)
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
|-----------------------------Gnd-9V--|
```
**Note:** [Pin #] as on drawing
| Pin # | Function |
|--:|---|
|1| batt sense |
|2| 3.3V |
|3| flow trig |
|4| gnd |
|5| LED (pwm)|
|6| start trig|
|7| allow water |
|8| bit 2 |
|9| bit 1|
|10| bit 0 |
|11| valve 0V/9V |
|12| valve 9V/0V |

# Notes
file:///home/bastien/while-true/common/garden-reporter/water-control/index.html?period=1&start-time=12%3A30&duration=66

# Bibliography
 * [Arduino-Nano-33-IoT-Ultimate-Guide](https://github.com/ostaquet/Arduino-Nano-33-IoT-Ultimate-Guide)
 * [Official](https://docs.arduino.cc/hardware/nano-33-iot)