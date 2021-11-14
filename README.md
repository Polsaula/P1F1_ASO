# P1F1_ASO

_This is the first part (of two) of a univesity project where I'be been asked to implement a Loadable Kernel Module (LKM) on a Raspberry Pi_

### Description

Please note that the folder "practiceLKMs" was for practice purpouses. The final project is inside "P1F1".
 
This LKM is designed to read values from 4 buttons and write 2 LEDs using the GPIO pins from the Rasperry Pi Zero WH. 
Each button has a unique functionality when pressed: turn on/off one of the LEDs.

### Pre-instalation 
It is important to keep your Raspberry Pi OS and packages up to date :
```
sudo apt update
sudo apt full-upgrade
```
In order to use this LKM, kernel headers must be installed:
```
sudo apt install raspberrypi-kernel-headers
```

### Instalation 

Once the folder is downloaded on your Raspberry Pi, move to "P1F1":
```
cd P1F1
```
Build the Kernel Object:
```
make
```
And then install it:
```
make install
```

### Termination

First remove the LKM:
```
make remove
```
And then remove all unwanted files generated on the project folders:
```
make clean
```
