#!/bin/bash

TARGET=$1
SERVER=$2

rm *_$TARGET_$SERVER

#echo "$TARGET" >> lifetime_$TARGET_$SERVER
#echo "$TARGET" >> prr_$SERVER
./lifetime.sh $TARGET >> lifetime_$TARGET_$SERVER
./prr.sh $TARGET >> prr_$TARGET_$SERVER
./delay.sh $TARGET >> delay_$TARGET_$SERVER

./avr.sh lifetime $TARGET >> avr_lifetime_$TARGET_$SERVER
./avr.sh prr $TARGET >> avr_prr_$TARGET_$SERVER
./avr.sh delay $TARGET >> avr_delay_$TARGET_$SERVER
