# rkdeveloptool
rkdeveloptool gives you a simple way to read/write rockusb device.let's start.

## Supported subsystems 

1. Linux
1. MacOS X, 11
1. Windows ( MinGW-W64 )

## compile and install

1. install libusb and libudev (libudev should be skipped for MinGW-W64 and MacOS )
  * linux:
` sudo apt-get install libudev-dev libusb-1.0-0-dev dh-autoreconf `
  * MacOS ( with homebrew ):
` brew install libusb `
  * MinGW-W64 ( x86.64 ):
` pacman -S mingw-w64-x86_64-libusb `
2. go into root of rkdeveloptool
2. `aclocal`
2. `autoreconf -i`
2. `autoheader`
2. `automake --add-missing`
2. `./configure`
2. `make`

## Features
rkdeveloptool usage,input "rkdeveloptool -h" to see
```
example:
1.download kernel.img
sudo ./rkdeveloptool db RKXXLoader.bin    //download usbplug to device
sudo ./rkdeveloptool wl 0x8000 kernel.img //0x8000 is base of kernel partition,unit is sector.
sudo ./rkdeveloptool rd                   //reset device
```

## compile error help
if you encounter the error like below:
* ./configure: line 4269: syntax error near unexpected token `LIBUSB1,libusb-1.0'
* ./configure: line 4269: `PKG_CHECK_MODULES(LIBUSB1,libusb-1.0)'

You should install pkg-config libusb-1.0:
 ` sudo apt-get install pkg-config libusb-1.0 ` 

## Static build required for Windows
* MinGW-W64 may need to build binary as static.
* Proceed step to configure, then edit Makefile.
* Find CXXLINK and CLINK, ( may placed on each 140 and 153 line )
* insert `-static` before `-o`.
* then build it.
