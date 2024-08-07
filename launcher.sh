#!/bin/bash

DIR=/tmp/mtacoin

SERVER=alonag/mtacoin:server
MINER=alonag/mtacoin:miner

DIFFICULTY=20

mkdir -p $DIR

sudo touch mtacoin.config

echo "DIFFICULTY=23" | sudo tee $DIR/mtacoin.config

docker pull $SERVER

docker pull $MINER

docker run  -v $DIR:/mnt/mta -d $SERVER

docker run  -v $DIR:/mnt/mta -d $MINER

docker run  -v $DIR:/mnt/mta -d $MINER

#docker ps >  get [container id] > docker exec -it [container id] bash > tail -f /var/log/mtacoin.log