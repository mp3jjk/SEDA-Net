#!/bin/bash
NODE=$1

sudo make sniff_test.upload NODEID=0x05 TARGET=zoul BOARD=firefly PORT=/dev/ttyUSB$NODE
