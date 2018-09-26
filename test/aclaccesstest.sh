#!/usr/bin/env bash
for i in `seq 0 7`
do
    for j in `seq 0 7`
    do
        echo "CHECKING 4${i}${j}"
        sudo chmod 4${i}${j} /home/acluser/source.access
        ../get /home/acluser/source ../dest
    done
done
