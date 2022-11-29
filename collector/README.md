# Introduction
The collector receives data from probes via an USB/Serial connection to an XBEE s2c 802.15.4.
Then interprets the data and apply calibration, on a per probe basis.
Finally it records the data in influxdb.
The data that are recorded are :
 * probe id (64 bit source address)
 * rssi (RF link level), from both sites
 * battery level
 * soil moisture
 * temperature
 * light

The collector allows to connect to a TCP port
Using negotiation, a specific endpoint may be kept awake
Then the TCP just forwards serial
Client can connect, negotiate, and use socat to forward to local pty
The local pty can be used by XCTU

The data flow follows
```
Xbee --> Serial/USB --> python3 --> influxdb
```

# Notes
 * only poll ADC
  - awakening is 0.2, and moisture read looks ok
 * set time before sleep to 0.1
 ==> All that should give a factor of:
 25 days (measured)
 / 0.62 (measured 62% of battery used)
 x 15/10 (10 to 15 min cycle)
 x 1.8 (sec old awakening + 1 sec time before sleep)
 / 0.16 (sec awakening max 0.06 + 0.1 time before sleep)
 = 680 days

# TODO
 * querry local xbee ?
  - volt, status, err, etc
    Local AT Command Request - 0x08 page 133
 * check C8 value, page 83
 * check A1 value, should NOT use association NOR scan channels
   page 92
 * check A2 value, coord side, do ot allow association
 * check power config, PL:4 PM:1
 * Check CCA theshold, CA, default 0x2C, range 0x28 0x50
   Europ they say 0x34, page 99
   if energy above that, no tx
   ? Put it higher value, with * -1 it means lower
 * Sleep Option, SO
   Use 0b01
   1 for enable sample on wakeup
   0 for no poll for data on waking
 * IC should be 0
   check other flags to disable on digital IO
 * IR set to 0
 
 * D6 collector, enable RTS ?
 * IT set to 4 ?
 * CH now 19, use 1A ?
   Should be same, coz both have no conflicts and show same noise floor in SA mode
 

# Deploy
```bash
docker build . -t garden-collector

## TODO make a garden-network with influxdb

# if influxdb is on host
docker run -d --restart always --name garden-collector --group-add dialout --network host --env-file env_file --device=/dev/ttyUSB0 garden-collector

# else
docker run -d --restart always --name garden-collector --group-add dialout -p 8087:8087 --env-file env_file --device=/dev/ttyUSB0 garden-collector
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
 * setup End Point as :
   - Cycle Sleep
   - IR = 0
     millisec between samples. IR = 0 means no auto-sampling
   - keep SO (Sleep Option) default
     default 0b00, 0b01 disable wakeup poll, 0b10 no sample on wakeup
   - Use 64bit address, coz 64bit address is required to send config
     MY should be set to 0xFFFF
   - endpoint destination (DH and DL) should be set to collector MAC
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
 * 0x17 Remote AT Command Request, p137
 * 0x97 Remote AT Command Response, p155
 * 0x82 64-bit I/O Sample Indicator, p144
 * Channel (CH) B seams to have better RSSI

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
# install
# env-file contains basic setups
docker run -d --restart always -p 8086:8086 \
    --env-file env_file_install \
    -v garden-reporter-influxdb:/var/lib/influxdb2 \
    --name garden-reporter-influxdb influxdb

# docker stop/rm/start garden-reporter
# ... have persistent data with named volume

# config with
docker exec -it garden-reporter-influxdb influx

# backup
# from bastien@gnome-server:~/dev/garden-reporter/collector$ 
bckdate=$(date +%y_%m_%d-%H_%M_%S)
docker exec -it garden-reporter-influxdb bash -c "influx backup -b $BUCKET -t $INFLUX_TOKEN /tmp/$bckdate"
docker cp garden-reporter-influxdb:/tmp/$bckdate ./backup_influxdb/$bckdate

# restore
docker cp ./backup_influxdb/$bckdate garden-reporter-influxdb:/tmp/backup
# --full also restores dash boards, tokens, etc
docker exec -it garden-reporter-influxdb bash -c "influx restore /tmp/backup --full"

```

## grafana
```bash
docker run -d --restart always -p 3000:3000 -v grafana:/var/lib/grafana --name garden-reporter-grafana grafana/grafana
```

# TCP Serial
A TCP port is open to allow taking over the serial for a specific device MAC.
It first wait for the target to wake up, and disable cycle sleep
It then mounts the tcp connection as a pty, to open with xctu
When the tcp connection is broken, the device is put back to cycle sleep mode, and the pty is unmounted
```bash
pipenv run python3 ./source/serial_tcp_client.py
```

