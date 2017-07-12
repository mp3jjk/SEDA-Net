#!/bin/bash

JOONKI=0

if [ $JOONKI -eq 0 ]
then
    CONTIKI=/media/user/Harddisk/Double-MAC/
else
    CONTIKI=~/Desktop/Double-MAC/
fi

echo "Short range simulation"
sed -i 's/\#define DUAL_RADIO 1/\#define DUAL_RADIO 0/g' $CONTIKI/platform/cooja/contiki-conf.h
sed -i 's/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 1/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 0/g' $CONTIKI/platform/cooja/contiki-conf.h


topology=$1
TRAFFIC_MODEL=$2
PERIOD=$3
ARRIVAL_RATE=$4
ALPHA=$5
STROBE_CNT=$6
LONG_WEIGHT=1
LSA_R=0
LR_range=2X
PARENT_REDUCTION=0
REDUCTION_RATIO=0
DATE=$7
DATA_ACK=$8
ALPHA_DIV=$9

if [ $TRAFFIC_MODEL -eq 0 ]
then
    mkdir $DATE\_traffic$TRAFFIC_MODEL\_period$PERIOD\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
    cd $DATE\_traffic$TRAFFIC_MODEL\_period$PERIOD\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
else
    mkdir $DATE\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
    cd $DATE\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
fi

../param.sh $LONG_WEIGHT $ALPHA $STROBE_CNT $LSA_R $TRAFFIC_MODEL $PERIOD $ARRIVAL_RATE $PARENT_REDUCTION $REDUCTION_RATIO $DATA_ACK 0 $ALPHA_DIV

if [ ! -e sr\_$topology ]
then
    mkdir sr\_$topology\_strobe$STROBE_CNT
fi
cd sr\_$topology\_strobe$STROBE_CNT
echo "#########################  We are in $PWD  ########################"

if [ ! -e COOJA.testlog ]
then
    java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/0502_$topology\_$LR_range\.csc -contiki="$CONTIKI"
#    java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/0601_$topology\_$LR_range\_loss\.csc -contiki="$CONTIKI"
fi
../../pp.sh
cd ../..

echo "Simulation finished"
