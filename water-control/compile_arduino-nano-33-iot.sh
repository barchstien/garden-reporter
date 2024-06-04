#!/bin/bash

PATH_ARDUINO_CLI="/home/bastien/while-true/common/arduino-ide_2.3.2_Linux_64bit/resources/app/lib/backend/resources/arduino-cli"

# TODO add -u and specify port to upload to
$PATH_ARDUINO_CLI compile -b arduino:samd:nano_33_iot ./water-control/