#!/bin/bash

make clean TARGET=zoul
make sniff_test TARGET=zoul BOARD=firefly
sudo make sniff_test.upload NODEID=0x01 TARGET=zoul BOARD=firefly PORT=/dev/ttyUSB$1
