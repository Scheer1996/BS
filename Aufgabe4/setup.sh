#!/bin/bash

module="aufgabe"
device="aufgabe"
mode="666"

sudo /sbin/insmod ./$module.ko $* || exit 1

bash filter.sh

#sudo tail -f /var/log/kern.log

sudo rm /dev/$device

major=$(grep $module /proc/devices | cut -c-4)

sudo mknod /dev/$device c $major 0

sudo chmod $mode /dev/$device

#echo Ein device fuer major nummer $major wurde erstellt
#mknod /dev/${device}f c $major 
