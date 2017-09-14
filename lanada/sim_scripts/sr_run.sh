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
LR_range=${11}
PARENT_REDUCTION=0
REDUCTION_RATIO=0
DATE=$7
DATA_ACK=$8
ALPHA_DIV=$9
ONLY_LONG=${10}
CHECK=${12}

if [ $TRAFFIC_MODEL -eq 0 ]
then
    mkdir $DATE\_traffic$TRAFFIC_MODEL\_period$PERIOD\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
    cd $DATE\_traffic$TRAFFIC_MODEL\_period$PERIOD\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
else
    mkdir $DATE\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
    cd $DATE\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_alpha$ALPHA\_$ALPHA_DIV\_dataack$DATA_ACK
fi

../param.sh $LONG_WEIGHT $ALPHA $STROBE_CNT $LSA_R $TRAFFIC_MODEL $PERIOD $ARRIVAL_RATE $PARENT_REDUCTION $REDUCTION_RATIO $DATA_ACK 1 $ALPHA_DIV $ONLY_LONG $CHECK 0

if [ ! -e $topology\_sr\_strobe$STROBE_CNT\_L$ONLY_LONG\_$LR_range\_$CHECK ]
then
    mkdir $topology\_sr\_strobe$STROBE_CNT\_L$ONLY_LONG\_$LR_range\_$CHECK
fi
cd $topology\_sr\_strobe$STROBE_CNT\_L$ONLY_LONG\_$LR_range\_$CHECK
echo "#########################  We are in $PWD  ########################"

HERE=$PWD
cd $CONTIKI/lanada
make clean TARGET=cooja
cd $HERE

if [ ! -e COOJA.testlog ]
then
	# cd $CONTIKI/tools/cooja
    if [ $ONLY_LONG -eq 0 ]
    then
	 java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\.csc -contiki="$CONTIKI"
#	java -mx512m -classpath $CONTIKI/tools/cooja/apps/mrm/lib/mrm.jar: -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/0729_$topology\_$LR_range\.csc -contiki="$CONTIKI"
	# ant run_nogui -Dargs=/home/user/Desktop/Double-MAC/lanada/sim_scripts/scripts/0729_36grid_2X.csc -Ddir=$PWD
    else
	 java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_L\.csc -contiki="$CONTIKI"
#	ant run_nogui -Dargs=/home/user/Desktop/Double-MAC/lanada/sim_scripts/scripts/0729_36grid_2X.csc
    fi
	# cd $HERE
fi
../../pp.sh
cd ../..

echo "Simulation finished"
