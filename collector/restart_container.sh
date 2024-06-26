#!/bin/bash

NAME="garden-collector"

docker container stop $NAME
docker container rm $NAME
# original
#docker run -d --restart always --name $NAME --group-add dialout --network host --env-file env_file --device=/dev/ttyUSB0 --log-opt max-size=50m garden-collector

# test/debug
docker run -d --restart always --name $NAME --group-add dialout --network host --env-file env_file --device=/dev/ttyUSB0 --log-opt max-size=50m garden-collector-dev