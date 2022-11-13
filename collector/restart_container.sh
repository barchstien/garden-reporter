#!/bin/bash

NAME="garden-collector"

docker container stop $NAME
docker container rm $NAME
docker run -d --restart always --name $NAME --group-add dialout --network host --env-file env_file --device=/dev/ttyUSB0 garden-collector
