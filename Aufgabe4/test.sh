#!/bin/bash

destination="/dev/aufgabe"

cat < $destination
sleep 1
echo "Hallo" > $destination
cat < $destination
sleep 5
echo "Test" > $destination
cat < $destination
sleep 4
echo "123" > $destination
cat < $destination
sleep 3
echo "12" > $destination
cat < $destination
sleep 2
echo "1" > $destination
cat < $destination
sleep 1
echo "OMG IT WORKS" > $destination
cat < $destination

dmesg | grep Aufgabe
