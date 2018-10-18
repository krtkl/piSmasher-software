# piSmasher Softare Projects

This directory contains the common library code (in 'lib') and the configuration application code. The application code is located in each application directory. The makefile within this directory will build all application code and place the resulting executables in the 'build' directory.

## Simple HDMI Configuration Script
The 'hdmi_en.sh' can be run after the project executables have been build with 'make'. This script will configure the HDMI transceivers and the video pipeline within the programmable logic. The configuration will pass-through the received HDMI signal from the HDMI receiver through a video test pattern generator and out of the HDMI transmitter. The video format will be set to 1080p@60 with a 24-bit RGB video colorspace.

```
$ ./hdmi_en.sh
```
