#!/usr/bin/env bash
for i in `seq 0 7`
do
    echo "checking ${i}00 mode"
    sudo chmod ${i}00 /home/acluser/source.access
    ../get /home/acluser/source ../dest
done
sudo chmod 700 /home/acluser/source.access
for i in `seq 0 7`
do
    echo "checking ${i}00 mode"
    sudo chmod ${i}00 /home/acluser/source
    ../get /home/acluser/source ../dest
done