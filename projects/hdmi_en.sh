#!/bin/bash

echo 958 > /sys/class/gpio/export
echo 959 > /sys/class/gpio/export

echo out > /sys/class/gpio/gpio958/direction
echo out > /sys/class/gpio/gpio959/direction

echo 1 > /sys/class/gpio/gpio958/value
echo 1 > /sys/class/gpio/gpio959/value

sleep 2

build/hdmi-config -m 1920x1080
build/uio-vtc -m 1920x1080 12
build/vid-tpg-config -w 1920 -h 1080 -b 0 13
