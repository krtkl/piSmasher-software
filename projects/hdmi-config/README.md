# HDMI Receive/Transmit Configuration

Configures the HDMI receiver and transmitter for 24-bit RGB color space and configures the transmitter to output a specified video format. The video format is specified using an optional mode argument '-m' followed by the desired output format. Video formats supported by this program are: 

* 1280x720
* 1366x768
* 1920x1080

This program uses the I2C device to configure the HDMI transceivers and thus must be run with root permissions.

```
admin@pismasher:~$ sudo ./hdmi-config [-m mode]
```

