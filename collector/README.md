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

The collector allows to connect to a TCP port, that can be mounted to a local virtual serial port, allowing to use XCTU (Xbee official utilities) via the network. More in [TCP Serial chapter](#tcp-serial)

The data flow follows
```
Xbee --> Serial/USB --> python3 --> influxdb
```

**TODO** a nice diagram of collector guts

## Features
 - Waits for probes (Xbee) to wake up, and trigger sampling
 - Apply calibration
 - Store collected data to influx db
 - Keep a list of probes and their profiles
 - Water Web Server, for Human interaction, and water-control

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

## Calibration
It may use a formula from datasheet or a manual calibration
 * moist : get dry/immerge value, gives linear ratio as %
 * temperature : from datasheet deg = V*100 - 50
 * Compare with a calibrated lux meter
 * battery voltage divider = 6

## Water Web Server
It has 2 use cases :
 * For Human, to setup auto-watering, and get short status of water-control
 * For water-control, to poll config and push status

### water-control /report
water-control makes a GET report to **/report**
GET params are :
 * last scheduled watering (?)
 * next schedule watering
 * battery voltage
 * water liter value
 * watering now
 * uptime
Returns **application/json** with params:
 * 'period_day_value': the period at which watering occurs
 * 'start_time_hour_minute_value': watering start time
 * 'duration_minute_value': duration of the watering in minutes
 * 'sec_since_1970': current time in sec since EPOCH

### water-control /debug
water-control makes a GET debug to **/debug**, with params :
 * 'debug': text


# TCP Serial
A TCP port is open to allow taking over the serial for a specific device MAC.
It first wait for the target to wake up, and disable cycle sleep
It then mounts the tcp connection as a pty, to open with xctu
When the tcp connection is broken, the device is put back to cycle sleep mode, and the pty is unmounted
```bash
pipenv run python3 ./source/serial_tcp_client.py

# run XCTU
/opt/Digi/XCTU-NG/app
```

# Deploy
Use docker compose with compose.yaml
It's about doing :
```bash
docker compose up --build -d
```
With extra few steps for 1st time deploy 

1. create env_file  
    Used to set some const and secrets  
    expects **env_file** as :
    ```bash
    # used by collector
    export INFLUX_TOKEN=should-be-generated-with-influx-auth-later
    
    # used by influxdb and collector
    export ORG=potager-org
    export BUCKET=potager-bucket
    
    # used by influxdb
    export USERNAME=potager
    # 8 character minimum
    export PASSWORD=potagerpotager
    ```

2. start/config influxdb
    ```bash
    # start influxdb
    docker compose up influxdb -d
    
    # setup influxdb
    source env_file
    docker exec garden-reporter-influxdb influx setup \
      --username $USERNAME \
      --password $PASSWORD \
      --org $ORG \
      --bucket $BUCKET \
      --force
    
    # generate token
    docker exec garden-reporter-influxdb influx auth create --all-access --org $ORG
    # copy/paste token to env_file
    # list token with 
    docker exec garden-reporter-influxdb influx auth list
    ```
    See [influxdb docker official](https://hub.docker.com/_/influxdb)

3. start/config grafana
   ```bash
   docker compose up grafana -d
   ```
   Got to web ui http://192.168.1.X:3000  
   Default pass is admin:admin  
   Add :
   - an influxdb data source using the generated TOKEN  
     - use the real ip (not localhost) in url
     - Disable all Auth, only fill the token lower in the page
   - a user to access on daily basis
     - make the user admin to allow creating dashboard

4. create/update collector and water-web-server config 
    ```bash
    docker volume create garden-collector
    docker run --rm -it -v $(pwd):/tmp/garden-collector -v garden-collector:/var/lib/garden-collector ubuntu cp /tmp/garden-collector/collector.yaml /var/lib/garden-collector/
    docker run --rm -it -v $(pwd):/tmp/garden-collector -v garden-collector:/var/lib/garden-collector ubuntu cp /tmp/garden-collector/water-web-config.yaml /var/lib/garden-collector/
    # set rights
    docker run --rm -it -v garden-collector:/var/lib/garden-collector ubuntu chown -R 1000:1000 /var/lib/garden-collector
    ```

5. start collector
    ```bash
    docker compose up collector --build -d
    ```


# Maintenance
```sh
# compose and build Docker file
docker compose up collector --build -d
docker compose restart collector

# influxdb token
docker exec garden-reporter-influxdb influx auth list

# collector config
# copy yaml TO container (collector needs restart, not water web server)
docker cp water-web-config.yaml garden-collector:/var/lib/garden-collector/
docker cp collector.yaml garden-collector:/var/lib/garden-collector/
# copy yaml FROM container
docker cp garden-collector:/var/lib/garden-collector/water-web-config.yaml ./
docker cp garden-collector:/var/lib/garden-collector/collector.yaml ./
```

# Backup
TODO, use docker compose too !!!





# Modules

## xbee
Configure xbees with XCTU
Install and launch XCTU config SW
```bash
# launch XCTU
/opt/Digi/XCTU-NG/app

# serial tcp
/tmp/garden0
```
probe XCTU profile has sleep disable by default, it has to be enabled manually  
**WARNING** When sleeping, got to force reboot to reconnect, hard to reconnect  
**INFO** Using serial/TCP collector disable/re-enable sleep mode on tcp client connect/disconnect  

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

## Python Collector
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

### docker
Run collector in docker, see [./restart_container.sh](restart_container.sh)  
Using a named volume for retaining config. Especially required for water-web-control which read and write to config

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
# keep ? : --env-file ./env_file_influxdb \
docker run -d --restart always -p 8086:8086 \
    -v garden-reporter-influxdb:/var/lib/influxdb2 \
    --memory=4000m \
    --name garden-reporter-influxdb influxdb:2

# docker stop/rm/start garden-reporter
# ... have persistent data with named volume

# config with
docker exec -it garden-reporter-influxdb influx

# backup
# from bastien@gnome-server:~/dev/garden-reporter/collector$ 
bckdate=$(date +%y_%m_%d-%H_%M_%S)
docker exec -it garden-reporter-influxdb bash -c "influx backup -b $BUCKET -t $INFLUX_TOKEN /tmp/$bckdate/"
docker cp garden-reporter-influxdb:/tmp/$bckdate ./backup_influxdb/

# restore
docker cp ./backup_influxdb/$bckdate garden-reporter-influxdb:/tmp/backup
# --full also restores dash boards, tokens, etc
docker exec -it garden-reporter-influxdb bash -c "influx restore /tmp/backup --full"

```

## grafana
```bash
docker run -d --restart always -p 3000:3000 \
    -v grafana:/var/lib/grafana \
    --memory=5000m \
    --name garden-reporter-grafana grafana/grafana
```
### source config
All was working, then I had to re-laucnh to apply max use of RAM
Then grafana couldn't reach influxdb, complaining about IP .53 not reachable
... no-one has such IP
After loging on as admin, found influxdb config URL to be : http://garden-reporter-influxdb:8086
--> Directly set to host server IP (.66) --> working again !


# server crash investigations
```sh
bastien@gnome-server:~$ sudo apt upgrade 
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
Calculating upgrade... Done
The following packages have been kept back:
  python3-update-manager update-manager-core vim vim-common vim-runtime vim-tiny xxd
The following packages will be upgraded:
  ubuntu-advantage-tools ubuntu-pro-client ubuntu-pro-client-l10n
3 upgraded, 0 newly installed, 0 to remove and 7 not upgraded.
Need to get 241 kB of archives.
After this operation, 102 kB of additional disk space will be used.
Do you want to continue? [Y/n] 
Get:1 http://fr.archive.ubuntu.com/ubuntu jammy-updates/main amd64 ubuntu-pro-client-l10n amd64 32.3~22.04 [20.3 kB]
Get:2 http://fr.archive.ubuntu.com/ubuntu jammy-updates/main amd64 ubuntu-pro-client amd64 32.3~22.04 [209 kB]
Get:3 http://fr.archive.ubuntu.com/ubuntu jammy-updates/main amd64 ubuntu-advantage-tools all 32.3~22.04 [10.9 kB]
Fetched 241 kB in 0s (1222 kB/s)                   
Preconfiguring packages ...
(Reading database ... 117511 files and directories currently installed.)
Preparing to unpack .../ubuntu-pro-client-l10n_32.3~22.04_amd64.deb ...
Unpacking ubuntu-pro-client-l10n (32.3~22.04) over (31.2.3~22.04) ...
Preparing to unpack .../ubuntu-pro-client_32.3~22.04_amd64.deb ...
Unpacking ubuntu-pro-client (32.3~22.04) over (31.2.3~22.04) ...
Preparing to unpack .../ubuntu-advantage-tools_32.3~22.04_all.deb ...
Unpacking ubuntu-advantage-tools (32.3~22.04) over (31.2.3~22.04) ...
Setting up ubuntu-pro-client (32.3~22.04) ...
Installing new version of config file /etc/apparmor.d/ubuntu_pro_apt_news ...
Setting up ubuntu-pro-client-l10n (32.3~22.04) ...
Setting up ubuntu-advantage-tools (32.3~22.04) ...
Processing triggers for man-db (2.10.2-1) ...
Scanning processes...                                                                         
Scanning candidates...                                                                        
Scanning processor microcode...                                                               
Scanning linux images...                                                                      

Running kernel seems to be up-to-date.

Restarting services...
 systemctl restart containerd.service cron.service irqbalance.service multipathd.service packagekit.service polkit.service rsyslog.service snapd.service ssh.service systemd-journald.service systemd-networkd.service systemd-resolved.service systemd-timesyncd.service systemd-udevd.service udisks2.service upower.service
Job for systemd-timesyncd.service failed.
See "systemctl status systemd-timesyncd.service" and "journalctl -xeu systemd-timesyncd.service" for details.
Job for systemd-journald.service failed because the control process exited with error code.
See "systemctl status systemd-journald.service" and "journalctl -xeu systemd-journald.service" for details.
Job for udisks2.service failed because a fatal signal was delivered to the control process.
See "systemctl status udisks2.service" and "journalctl -xeu udisks2.service" for details.
Job for containerd.service failed because a fatal signal was delivered to the control process.
See "systemctl status containerd.service" and "journalctl -xeu containerd.service" for details.
Job for upower.service failed because the control process exited with error code.
See "systemctl status upower.service" and "journalctl -xeu upower.service" for details.
Job for systemd-resolved.service failed because of unavailable resources or another system error.
See "systemctl status systemd-resolved.service" and "journalctl -xeu systemd-resolved.service" for details.
Job for snapd.service failed because the control process exited with error code.
See "systemctl status snapd.service" and "journalctl -xeu snapd.service" for details.
Service restarts being deferred:
 systemctl restart ModemManager.service
 systemctl restart dbus.service
 systemctl restart docker.service
 systemctl restart getty@tty1.service
 systemctl restart networkd-dispatcher.service
 systemctl restart openvpn-server@server.service
 systemctl restart systemd-logind.service
 systemctl restart unattended-upgrades.service
 systemctl restart wpa_supplicant.service

No containers need to be restarted.

No user sessions are running outdated binaries.

No VM guests are running outdated hypervisor (qemu) binaries on this host.
debconf: DbDriver "config": could not write /var/cache/debconf/config.dat-new: Input/output error
E: Problem executing scripts DPkg::Post-Invoke 'if [ -d /var/lib/update-notifier ]; then touch /var/lib/update-notifier/dpkg-run-stamp; fi; /usr/lib/update-notifier/update-motd-updates-available 2>/dev/null || true'
E: Sub-process returned an error code
E: Sub-process [ ! -f /usr/lib/ubuntu-advantage/apt-esm-json-hook ] || /usr/lib/ubuntu-advantage/apt-esm-json-hook || true returned an error code (100)
E: Failure running hook [ ! -f /usr/lib/ubuntu-advantage/apt-esm-json-hook ] || /usr/lib/ubuntu-advantage/apt-esm-json-hook || true
bastien@gnome-server:~$ 
bastien@gnome-server:~$ 
bastien@gnome-server:~$ 
bastien@gnome-server:~$ 
bastien@gnome-server:~$ htop
htop: command not found
bastien@gnome-server:~$ top
top: command not found
bastien@gnome-server:~$ sudo shutdown -r now
sudo: error while loading shared libraries: libsudo_util.so.0: cannot open shared object file: Input/output error
bastien@gnome-server:~$ 
bastien@gnome-server:~$ 

```