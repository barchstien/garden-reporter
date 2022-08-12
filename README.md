# Overview
```
     ┌───┐
     │ P ├────────────┐
     └───┘            │
┌───┐                ┌▼───────────────────┐
│ P ├────────────────►     Collector      │
└───┘                └▲───┬───────────────┤
       ┌───┐          │   │time series db │
       │ P ├──────────┘   ├───────────────┤
       └───┘              │ visualisation │
                          └───────────────┘
```
Probes (P) report to collector via rf for record and display

Probe for :
 * soil humidity
 * air humidity
 * air temperature
 * luminosity
 * 6 month+ battery

Report to collector
 * via xbee 2.4GHz 802.15.4
 * every 15min using cycle sleep

Collector
 * xbee usb
 * rapsberry pi (or low power unit)
 * storage (raid/cloud backup)
 * influxdb
 * grafana
