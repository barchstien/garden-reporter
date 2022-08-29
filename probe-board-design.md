# Overview
![probe overview](/resources/probe-board-overview.png)

**Note :** [XXX-XX] : mouser order number

# Components
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
   TODO add Resistor between gate and Gnd, to help discharge ?
   |---> Or not ? coz xbeen will drive it low
   |---> keep un-populated pad for resistor
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

## Temp sensor
 * TMP36GT9Z [584-TMP36GT9Z] 1.81E
   Must add O.1uF cap, very close to device, ie on wire

## Photo transistor
 * TEPT5700 [782-TEPT5700]
   Use with 10K resistor, measure U, then I = U/R
   From I, get Lux

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



