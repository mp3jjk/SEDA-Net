#!/bin/bash

SERVER=$1

rm lifetime_$1
rm prr_$1

echo "G5" >> lifetime_$1
echo "G5" >> prr_$1
./lifetime.sh G5 >> lifetime_$1
./prr.sh G5 >> prr_$1

echo "G6" >> lifetime_$1
echo "G6" >> prr_$1
./lifetime.sh G6 >> lifetime_$1
./prr.sh G6 >> prr_$1

echo "G7" >> lifetime_$1
echo "G7" >> prr_$1
./lifetime.sh G7 >> lifetime_$1
./prr.sh G7 >> prr_$1

echo "G8" >> lifetime_$1
echo "G8" >> prr_$1
./lifetime.sh G8 >> lifetime_$1
./prr.sh G8 >> prr_$1
