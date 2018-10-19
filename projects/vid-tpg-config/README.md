# Video Test Pattern Generator Control

This utility configures the video test pattern generator IP. Optional arguments are available for setting the video frame height (in rows), width (in pixels) and background pattern ID. This utility uses a UIO device to access the IP register space for configuration and must be run with root permissions. The UIO device ID that should be the last argument (required) of the program invocation. The default configuration will output 1080 lines of 1920 pixels using RGB format and passing through the video input source as the test pattern output.

```
admin@pismasher:~$ sudo ./vid-tpg-config [-h height] [-w width] [-b pattern] UIO_ID
```
The following table describes the pattern IDs that can be used as the background pattern option argument:

| ID | Pattern Output             |
|----|----------------------------|
| 0  | Video input (default)      |
| 1  | Horizontal ramp            |
| 2  | Vertical ramp              |
| 3  | Temporal ramp              |
| 4  | Solid red                  |
| 5  | Solid green                |
| 6  | Solid blue                 |
| 7  | Solid black                |
| 8  | Solid white                |
| 9  | Color bars                 |
| 10 | Zone plate                 |
| 11 | Tartan bars                |
| 12 | Cross hatch                |
|	13 | Color sweep                |
|	14 | Horiz. and vert. ramp      |
| 15 | Checkerboard               |
|	16 | Pseudorandom               |
|	17 | DisplayPort color ramp     |
|	18 | DisplayPort vertical lines |
| 19 | DisplayPort color square   |
