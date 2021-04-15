# rkdeveloptool-libusbk

This rkdeveloptool is focused on supporting Windows subsystem with libusbk.

## Supported subsystems 

1. Windows ( MinGW-W64 )

## compile and install

1. install libusbk developer version with same directory level with rkdeveloptool-libusbk.
2. `make`

## Features

1. rkdeveloptool usage,input "rkdeveloptool -h" to see example:
1. download kernel.img on Windows cmd.exe (need administrator privilege)
```
rkdeveloptool db RKXXLoader.bin    //download usbplug to device
rkdeveloptool wl 0x8000 kernel.img //0x8000 is base of kernel partition,unit is sector.
rkdeveloptool rd                   //reset device
```
