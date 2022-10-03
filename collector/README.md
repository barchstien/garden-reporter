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
bastien@gnome-server:~/dev/garden-reporter/collector$ docker exec -it garden-reporter-influxdb bash -c "influx backup -b gr-bucket -t $INFLUX_TOKEN /tmp/$(date +%y_%m_%d-%H_%M_%S)"
docker cp garden-reporter-influxdb:/tmp/* ./backup_influxdb/

```

## grafana
```bash
docker run -d --restart always -p 3000:3000 -v grafana:/var/lib/grafana --name garden-reporter-grafana grafana/grafana
```



