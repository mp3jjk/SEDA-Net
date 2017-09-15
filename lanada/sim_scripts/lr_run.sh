#!/bin/bash

JOONKI=0

if [ $JOONKI -eq 0 ]
then
    CONTIKI=/media/user/Harddisk/Double-MAC
else
    CONTIKI=~/Desktop/Double-MAC
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
CHECK=${16}
LSA_ENHANCED=${17}

if [ $TRAFFIC_MODEL -eq 0 ]
then
    mkdir $DATE\_traffic$TRAFFIC_MODEL\_period$PERIOD\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
    cd $DATE\_traffic$TRAFFIC_MODEL\_period$PERIOD\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
else
    mkdir $DATE\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
    cd $DATE\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
fi

../param.sh $LONG_WEIGHT $ALPHA $STROBE_CNT $LSA_R $TRAFFIC_MODEL $PERIOD $ARRIVAL_RATE $PARENT_REDUCTION $REDUCTION_RATIO $DATA_ACK $LSA_MAC $ALPHA_DIV $CHECK $LSA_ENHANCED

if [ ! -e $topology\_lr\_weight$LONG_WEIGHT\_LR_range$LR_range\_check$CHECK\_strobe$STROBE_CNT\_lsa$LSA_R\_lsa_mac$LSA_MAC\_lsa_en$LSA_ENHANCED ]
then
    mkdir $topology\_lr\_weight$LONG_WEIGHT\_LR_range$LR_range\_check$CHECK\_strobe$STROBE_CNT\_lsa$LSA_R\_lsa_mac$LSA_MAC\_lsa_en$LSA_ENHANCED
fi
cd $topology\_lr\_weight$LONG_WEIGHT\_LR_range$LR_range\_check$CHECK\_strobe$STROBE_CNT\_lsa$LSA_R\_lsa_mac$LSA_MAC\_lsa_en$LSA_ENHANCED
echo "#########################  We are in $PWD  ########################"

HERE=$PWD
cd $CONTIKI/lanada
make clean TARGET=cooja
cd $HERE

if [ ! -e COOJA.testlog ]
then
#	cd $CONTIKI/tools/cooja
     java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\.csc -contiki="$CONTIKI"
#		ant run_nogui -Dargs=/home/user/Desktop/Double-MAC/lanada/sim_scripts/scripts/0729_36grid_2X.csc
#	cd $HERE
fi
../../pp.sh
cd ../..

echo "Simulation finished"
