#!/bin/bash

echo 958 > /sys/class/gpio/export
echo 959 > /sys/class/gpio/export

echo out > /sys/class/gpio/gpio958/direction
echo out > /sys/class/gpio/gpio959/direction

echo 1 > /sys/class/gpio/gpio958/value
echo 1 > /sys/class/gpio/gpio959/value

