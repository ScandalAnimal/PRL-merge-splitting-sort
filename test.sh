#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Nespravny pocet argumentov, spravny je 2"
    exit 1
fi

dd if=/dev/urandom bs=1 count=$1 of=numbers > /dev/null 2>&1

mpic++ --prefix /usr/local/share/OpenMPI -o mss mss.cpp

mpirun --prefix /usr/local/share/OpenMPI -np $2 mss $1

rm -f mss numbers
