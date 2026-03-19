#!/bin/bash
set -e

# Sync executable
bn=$(basename $1)
rpiaddr="ADRESSE DE VOTRE RASPBERRY PI ICI"

rsync -az $1/build/emulateurClavier $1/createurRequetes/createurRequetes "pi@$rpiaddr:/home/pi/projects/laboratoire5/"
# Execute GDB
ssh "pi@$rpiaddr" "nohup /home/pi/projects/laboratoire5/createurRequetes fichierComm 60 120 10 40 1 0 </dev/null >/home/pi/capture-createurRequetes 2>&1 & nohup sudo gdbserver :$3 /home/pi/projects/laboratoire5/emulateurClavier fichierComm 10000 10 > /home/pi/capture-stdout-$2 2> /home/pi/capture-stderr-$2 < /dev/null &"
sleep 1

