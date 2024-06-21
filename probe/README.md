# Overview
![probe overview](/resources/probe-board-overview.png)

## Features
 - Probe for :
   - soil humidity
   - air temperature
   - luminosity
   - battery level
 - 6 month+ battery (1 year). See **probe-board-power-use.ods**
 - Report to collector
   - via xbee 2.4GHz 802.15.4
   - every 15min using cycle sleep

Kicad files are in **probe/kicad** folder  
Components datasheet are in **probe/datasheet** folder  

## Board 3d view
![probe pcb 3d](/resources/probe-board-3d.png)

# Mechanic and water consideration
 - case should be wateproof
   - plastic bottles are inexpensive and water proof
     - bottom is filled with gravel for stability and in case of leak
   - sensors are attached to case
   - wires from sensors to probe board are water resistent
     - use plumbing goo/joint/paste
   - sensors are environment protected :
     - luminosity is already in epoxy case, wire nail polished
     - temp and wires are nail polished
     - soil moisture bottom is (home made) epoxy coated (water + abrasive)
     - soild moisture top is nail polished

# Components
**Note :** [XXX-XX] : mouser order number

## Xbee
 * XB24CAPIT-001, TH, s2c 802.15.4 [888-XB24CAPIT-001]
    - give 1uF and 8pf cap on supply
    - good keepout (bigger better, no gnd plane up), for pcb antenna
 * usb adapter
    - usb explorer board from sparkfun WRL-11812 [474-WRL-11812] 26.33E
    - dfrobot DFR0050 [426-DFR0050] 18.95E

## Switch
 * TN0702N3-G, can switch 20V, on at 1V, 0.1uA stdby  
   1.53E [689-TN0702N3-G]  
 * Use flywheel diode to protect mosfet from switched inductive load  
   SB130 [637-SB130]
 
## Battery 
 * 4x AA Nimh 2000mAh low self-discharged
   - Accus hybrides Aeonium - 2100mAh 8E for 4, 85% after 1 year
   - Energizer accu recharge extreme 2000mAh 8.6E for 4
 * 4x battery holder 58x31x28 12BH343D-GR [12BH343D-GR]
   OR 534-2476 with snap-on ?  
 Nimh nominal/min/max voltage 1.2/0.9/1.5  
 with 4 cells 4.8/3.6/6V 

## 3.3V voltage regulator
 * MCP1700-3302E/TO 0.5E [579-MCP1700-3302E/TO]  
   Vin 2.3 to 6V, Vdrop typical 178mV, max 350mV  
   250 mA max output  
   Use 1uF co at output (and 1 at input ?)  
 
## Soil moisture sensor
 * Capacitor sensor  
   SEN0193 DFRobot 7.9E [426-SEN0193]  
   SEN0308 DFRobot 15E  
   3.3V 5mA  
   **Warning** some cheapper alternative exists but aren't documented, nor reliable  
   Protect with nail polish

## Temp sensor
 * TMP36GT9Z [584-TMP36GT9Z] 1.81E  
   Must add O.1uF cap, very close to device, ie on wire

## Photo transistor
 * TEPT5700 [782-TEPT5700]  
   Use with 5K resistor for outside use with diffuser, measure U, then I = U/R  
   From I, get Lux, using calibration with a ref lux meter  
 The value of R will set the sensibility and saturation level  
 Use with a diffuser, to mitigate the effect of the angle towards the light source  
 
 For inside use, use a higher R value. 
 Aiming 5000 Lux max inside, which is 1/10 of outside max use
 Outside uses 5K --> use 50K --> only have 2x 20K

## Battery level sensor
 * Add battery level check with voltage divider  
   with 2x 10K resitor 1%  
   with zener diode 1N4620 3.3V, [610-1N4620BK]

## Connector
 * sensor to board, 3ways
  - 70543-0002 [538-70543-0002] pcb straigth header  
    70543-0107 for even more gold plating !
  - 79758-0011 [538-79758-0011] crimped wire, 30cm, 22awg
  - 50-57-9403 [538-50-57-9403  ] cable mount
 * battery to board, same with 2 pins
   [538-70543-0106]
 * Use kicad Molex_SL_171971
 
## Case
 * bicycle bottle, ~750mL, 20cm high, 7cm diameter

## Various
 * R 10K 1% metal film : [603-MFR-25FRF5210K]
 * c 10uF ceramic : [710-860010472002] 25VDC 85degC max
 * C 1uF ceramic : [810-FG18X7R1E105KRT0] 25VDC
 * c 8.2 pF ceramic : [80-C315C829D2G] 200VDC



