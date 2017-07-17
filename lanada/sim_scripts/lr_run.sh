#!/bin/bash

JOONKI=0

if [ $JOONKI -eq 0 ]
then
    CONTIKI=/media/user/Harddisk/Double-MAC/
else
    CONTIKI=~/Desktop/Double-MAC/
fi

echo "Long range simulation"
sed -i 's/\#define DUAL_RADIO 0/\#define DUAL_RADIO 1/g' $CONTIKI/platform/cooja/contiki-conf.h
sed -i 's/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 1/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 0/g' $CONTIKI/platform/cooja/contiki-conf.h

topology=$1
TRAFFIC_MODEL=$2
PERIOD=$3
ARRIVAL_RATE=$4
ALPHA=$5
STROBE_CNT=$6
LONG_WEIGHT=$7
LSA_R=$8
LR_range=$9
PARENT_REDUCTION=${10}
REDUCTION_RATIO=${11}
DATE=${12}
DATA_ACK=${13}
LSA_MAC=${14}
ALPHA_DIV=${15}

if [ $TRAFFIC_MODEL -eq 0 ]
then
    mkdir $DATE\_traffic$TRAFFIC_MODEL\_period$PERIOD\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
    cd $DATE\_traffic$TRAFFIC_MODEL\_period$PERIOD\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
else
    mkdir $DATE\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
    cd $DATE\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
fi

../param.sh $LONG_WEIGHT $ALPHA $STROBE_CNT $LSA_R $TRAFFIC_MODEL $PERIOD $ARRIVAL_RATE $PARENT_REDUCTION $REDUCTION_RATIO $DATA_ACK $LSA_MAC $ALPHA_DIV 0

if [ ! -e $topology\_lr\_weight$LONG_WEIGHT\_LR_range$LR_range\_strobe$STROBE_CNT\_lsa$LSA_R\_ratio$REDUCTION_RATIO\_lsa_mac$LSA_MAC ]
then
    mkdir $topology\_lr\_weight$LONG_WEIGHT\_LR_range$LR_range\_strobe$STROBE_CNT\_lsa$LSA_R\_ratio$REDUCTION_RATIO\_lsa_mac$LSA_MAC
fi
cd $topology\_lr\_weight$LONG_WEIGHT\_LR_range$LR_range\_strobe$STROBE_CNT\_lsa$LSA_R\_ratio$REDUCTION_RATIO\_lsa_mac$LSA_MAC
echo "#########################  We are in $PWD  ########################"

if [ ! -e COOJA.testlog ]
then
    java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\.csc -contiki="$CONTIKI"
fi
../../pp.sh
cd ../..

echo "Simulation finished"
