[TOC]

# Overview
![probe overview](/resources/overview.png)

Collect data about the garden (soil humidity, luminosity and temperature), that is gathered via xbee link to the collector, for storage and visualisation.

Control and measure wtaer flow. Water flows when it is programmed to, or when manually triggered for a set duration.

Xbee modules are configured to :
 - cycle sleep to probe every 15min and stay ON for around 100 msec
 - send data to the collector

## UI and Configuration
 * Collector is configured via YAML, see [collector](/collector/) folder
 * Probes are configured when deployed, by uploading the profile (see profile*.xpro files)
 * Visulisation via Grafana
 * Water-control polls the set program every time it reports water flow
 * Water flow program is configured via web UI

# Probe
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
More doc about probe in [probe/](./probe)

# Collector
## Features
 - Waits for probes (Xbee) to wake up, and trigger sampling
 - Apply calibration
 - Store collected data to influx db
 - Keep a list of probes and their profiles

## Probe profile
Probes have unit specific characteristics which need to be stored at the collector level :
 - Soil moisture dry/water values. Those values change from probe to probe, even more with (home made) epoxy coating
 - xbee identifier
 - placement
 - ganeric comments (dates for commissioning of probe and batteries)

## Design
 - xbee usb module
 - low power fanless PC (ASUS Mini PC PN41)
 - influxdb storage
 - grafana visulisation
 - python 3 (pipenv)
 - full yaml config

# IRL
## Visualisation
![IRL_grafana_4_days.png](/resources/IRL_grafana_4_days.png)
![IRL_grafana_30_days.png](/resources/IRL_grafana_30_days.png)
![IRL_grafana_battery.png](/resources/IRL_grafana_battery.png)
![IRL_grafana_rssi_diagnostics.png](/resources/IRL_grafana_rssi_diagnostics.png)
## Probe
![IRL_probe_board_IMG_20230210_194025.jpg](/resources/IRL_probe_board_IMG_20230210_194025.jpg)
![IRL_probe_IMG_20230729_140125.jpg](/resources/IRL_probe_IMG_20230729_140125.jpg)
![IRL_probe_IMG_20230729_140158.jpg](/resources/IRL_probe_IMG_20230729_140158.jpg)
## Collector
![IRL_collector_IMG_20230729_135918.jpg](/resources/IRL_collector_IMG_20230729_135918.jpg)
