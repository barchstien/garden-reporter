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

# SW
using Arduino IDE, source is in [water-control](./water-control)  
Requires Arduino Libraires (t on install with IDE) :  
 * ArduinoHttpClient (easy http GET)
 * ArduinoJson (Json http response)
 * Arduino_Low_Power (to confirm, to keep power usage to min)
 * RTCZero (No RTC module installed, would be a great addon)
 * Time (unix to date/time, local time sync)
 * WiFiNINA (wifi connection + web client)


# Notes
file:///home/bastien/while-true/common/garden-reporter/water-control/index.html?period=1&start-time=12%3A30&duration=66