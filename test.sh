#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Illegal number of arguments!"
    exit 1
fi

#vytvor soubor s cisly
dd if=/dev/urandom bs=1 count=$1 of=numbers

#preklad
mpic++ --prefix /usr/local/share/OpenMPI -o mss mss.cpp

#spusteni
mpirun --prefix /usr/local/share/OpenMPI -np $2 mss $1

#uklid
rm -f mss numbers
