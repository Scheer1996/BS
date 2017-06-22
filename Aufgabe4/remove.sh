#!/bin/sh

module="aufgabe"
device="aufgabe"

bash filter.sh

sudo /sbin/rmmod $module || exit 1

echo Der modul wurde entfernt

bash filter.sh

sudo rm -f /dev/$device
