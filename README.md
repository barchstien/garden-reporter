# Overview
![probe overview](/resources/overview.png)

Collect data about the garden (soil humidity, luminosity and temperature), that is gathered via xbee link to collector, for storage and visualisation.

Xbee modules shoud be configured to :
 - cycle sleep to probe every 15min and stay ON for 5sec max
 - send data to the collector

## Probe
![probe overview](/resources/probe-board-overview.png)
### Features
 - Probe for :
   - soil humidity
   - air temperature
   - luminosity
   - battery level
 - 6 month+ battery (1 year). See **probe-board-power-use.ods**
 - Report to collector
   - via xbee 2.4GHz 802.15.4
   - every 15min using cycle sleep

### Mechanic and water consideration
 - case should be wateproof
   - plastic bottles are inexpensive and water proof
     - bottom is filled with gravel for stability and in case of leak
   - sensors are attached to case
   - wires from sensors to probe board are water resistent
     - make an angle to avoid water drip
     - use plumbing goo/joint/paste
   - sensors are environment protected :
     - luminosity is already in epoxy case, wire nail polished
     - temp and wires are nail polished
     - soil moisture bottom is (home made) epoxy coated (water + abrasive)
     - soild moisture top is nail polished

![probe overview](/resources/probe-board-3d.png)

Kicad files are in **probe-kicad** folder  
Components datasheet are in **ref** folder

## Collector
### Features
 - Collect probes data
 - Store collected data
 - Visualise (web) collected data
 - Keep a list of probes and their profiles
 - Register generic events (probe placement change, weather, water consumption, etc)

### Probe profile
Probes have unit specific characteristics which need to be stored at the collector level :
 - Soil moisture dry/water values. Those values change from probe to probe, even more with (home made) epoxy coating
 - xbee identifier
 - placement
 - ganeric comments (dates for commissioning of probe and batteries)

### Design
 - xbee usb
 - rapsberry pi (or low power unit)
 - storage (raid/cloud backup)
 - influxdb
 - grafana
 - python
 - jupyter lab ?
 - docker ?
 - full yaml/json config
 - full yaml/json for probe profiles

#### influxdb playground
https://docs.influxdata.com/influxdb/v2.4/reference/sample-data/
```bash
docker network create --driver bridge influxdb-telegraf-net

# influxdb
# should used a named volume instead of mount
docker run -d --name=influxdb -p 8086:8086 -v /tmp/testdata/influx:/root/.influxdb2 --net=influxdb-telegraf-net influxdb

# telegraf
# set INFLUX_TOKEN as given in web ui
# set --config as given in web ui
docker run -d --name=telegraf --net=influxdb-telegraf-net --env INFLUX_TOKEN=PbVKP1f50nMWl9BfQCHA-e9NKtqvHi-6rceeJQ9ACMwmeH5dclucL6M_gkd4C4hgpaTrtLrPRoXJctcrLA3R-g== telegraf --config http://influxdb:8086/api/v2/telegrafs/09e09cfc1d337000
```

Screw telegraf, but use python directly to feed database
telegraf might be used for data replication or batch processing

## IRL
TODO pictures of probe and collector
