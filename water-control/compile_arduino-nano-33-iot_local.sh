#!/bin/bash

set -e 
set -o pipefail

PATH_ARDUINO_CLI="/home/bastien/while-true/common/arduino-ide_2.3.2_Linux_64bit/resources/app/lib/backend/resources/arduino-cli"

# TODO add -u and specify port to upload to
#$PATH_ARDUINO_CLI compile -b arduino:samd:nano_33_iot --output-dir ./bin/  ./water-control/
# -u upload
# --clean
$PATH_ARDUINO_CLI compile -b arduino:samd:nano_33_iot --clean -u -p /dev/ttyACM0 ./water-control/

# copy bin to rpi
#scp bin/water-control.ino.bin bastien@192.168.1.133:/home/bastien/upload/
# upload to board
#ssh bastien@192.168.1.133 "cd /home/bastien;./upload.sh"

# serial monitor
#ssh bastien@192.168.1.133 "cd /home/bastien;./bin/arduino-cli monitor -p /dev/ttyACM0"
