#!/bin/bash

TARGET=$1
SERVER=$2

rm lifetime_$TARGET_$SERVER
rm prr_$TARGET_$SERVER
rm delay_$TARGET_$SERVER

#echo "$TARGET" >> lifetime_$TARGET_$SERVER
#echo "$TARGET" >> prr_$SERVER
./lifetime.sh $TARGET >> lifetime_$TARGET_$SERVER
./prr.sh $TARGET >> prr_$TARGET_$SERVER
./delay.sh $TARGET >> delay_$TARGET_$SERVER
