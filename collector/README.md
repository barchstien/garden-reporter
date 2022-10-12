# Introduction
The collector receives data from probes via an USB/Serial connection to an XBEE s2c 802.15.4.
Then interprets the data and apply calibration, on a per probe basis.
Finally it records the data in influxdb.
The data that are recorded are :
 * probe id (16 bit source address)
 * rssi (RF link level)
 * battery level
 * soil moisture
 * temperature
 * light

The data flow follows
```
Xbee --> Serial/USB --> python3 --> influxdb --> grafana
```

# Modules

## xbee
Configure xbees with XCTU
Install and launch XCTU config SW
```bash
# launch XCTU
/opt/Digi/XCTU-NG/app
```
Load .xpro profiles for collector, and for each probe

### xbee config notes
 * Use 64bit address, coz 64bit address is required to send config
   MY should be set to 0xFFFF
 * Channel (CH) B seams to have better RSSI
 * endpoint destination (DH and DL) should be set to collector MAC
 * IR is millisec between samples. IR = 0 means no auto-sampling
 * SO Sleep option, default 0b00, 0b01 disable wakeup poll, 0b10 no sample on wakeup
 * 0x17 Remote AT Command Request, p137
 * 0x97 Remote AT Command Response, p155
 * 0x82 64-bit I/O Sample Indicator, p144
   |---> Replace by use of 0x92 ?
         legacy Digi RF products, does this apply to SC2 ?
 * Remote AT cmd of IS does not return RSSI...
   Use remote AT cmd DB (Last Packet RSSI)
   DO not use local, coz don't know from where last frame is from

#### Questions :
 * What about TX Request ?
 * use IS (Force Sample) ?
   p114 not much here, expect that it's an AT command
   |--> use 0x17 remote AT command with IS inside
        answered by 0x97
        https://www.digi.com/support/knowledge-base/digital-and-analog-sampling-using-xbee-radios
        AT command IS == 0x49 0x53
   |--> Remote AT cmd with IS
        7E 00 0F 17 01 00 13 A2 00 41 F2 61 50 FF FE 02 49 53 B3
   |--> Remote AT cmd DB (RSSI)
        7E 00 0F 17 01 00 13 A2 00 41 F2 61 50 FF FE 02 44 42 C9
   Takes 100 to 200msec to get response
 * for zigbee
   "Queried samples (IS) can be sent to sleeping End Devices. This is because the End Device’s 
   parent will buffer the IS command until the End Device’s next wake period. This is makes ZigBee 
   an ideal choice for remote sampling applications."
 * 0x97 AT cmd response frame ?
 * use IT to fit N sample per pkt ?
   |-> manual says that with short addr, 53 samples max per pkt
 * what is wakeup poll ?
 * BD (baud), increase to 115200 ?
   default 0x3 9600
   0x7 115200
 * EC (CCA failure), counter, what about pooling it ?
 * EA (ACK failure)

#### Conclusions
 * setup End Point as :
   - Cycle Sleep
   - IR = 0
   - keep SO default
   End Point send a sample on wake up
   Use this as a signal
   0x82 (RX (Receive) Packet 64-bit Address IO)
 * use RSSI from wake up sample frame
 * send remote AT cmd 
   - IS to get samples
   - DB for last RSSI
     ! Different from pervious RSSI
   - EA ACK failure cnt
     ! For collector too
   - EC CCA failure cnt
     ! collector too

## Python
Use **pipenv** to manage virtual env and packages
```bash
# load Pipfile
pipenv install
# load ./Pipfile.lock
pipenv install --ignore-pipfile
# add --dev for dev only pkg
# lock (save) current env to Pipfile.lock
pipenv lock

# add more pkg with
# use --dev if it's a dev only dep
pipenv install|uninstall blabla

pipenv shell
pipenv run <cmd>
```

### influxdb token and secrets
```bash
# export influx API token
# env_file line format is foo=bar
source env_file
export INFLUX_TOKEN
export ORG
export BUCKET

# OR use convenient script for it
source dev_setup_env.sh

# run
pipenv run python3 collector.py
```

### dev
```bash
# run detached
nohup pipenv run python3 main.py > log.txt &

# fwd serial to tcp
socat tcp-listen:8087,reuseaddr file:/dev/ttyUSB0,nonblock,b9600,raw,echo=0 
# virtual serial
socat pty,link=/tmp/garden0,raw,echo=0 tcp:192.168.1.66:8087
```

## influxdb
```bash
docker run -d --restart always -p 8086:8086 -v influxdb:/var/lib/influxdb --name garden-reporter-influxdb influxdb
# config with
docker exec -it influxdb influx

# backup
bastien@gnome-server:~/dev/garden-reporter/collector$ 
docker exec -it garden-reporter-influxdb bash -c "influx backup -b $BUCKET -t $INFLUX_TOKEN /tmp/$(date +%y_%m_%d-%H_%M_%S)"
docker cp garden-reporter-influxdb:/tmp/* ./backup_influxdb/

```

## grafana
```bash
docker run -d --restart always -p 3000:3000 -v grafana:/var/lib/grafana --name garden-reporter-grafana grafana/grafana
```



