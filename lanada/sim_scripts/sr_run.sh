#!/bin/bash

JOONKI=0

if [ $JOONKI -eq 0 ]
then
    CONTIKI=/media/user/Harddisk/Developing-Dual-Net/
else
    CONTIKI=~/Developing-Dual-Net/
fi

echo "Short range simulation"
sed -i 's/\#define DUAL_RADIO 1/\#define DUAL_RADIO 0/g' $CONTIKI/platform/cooja/contiki-conf.h
sed -i 's/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 1/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 0/g' $CONTIKI/platform/cooja/contiki-conf.h

topology=$1

TRAFFIC_MODEL=$2
PERIOD=$3
ARRIVAL_RATE=$4

LR_range="2X"
LONG_WEIGHT=1
ETX_WEIGHT=$5
BETA=$6
BETA_DIV=$7
CROSS_OPT=$8

STROBE_CNT=$9
CHECK=${10}

DATE=${11}

ONLY_LONG=0
SEED_NUMBER=${12}
MRM=${13}
LT_PERCENT=${14}
LTMAX=${15}

sed -i "11s/.*/    <randomseed>$SEED_NUMBER<\/randomseed>/" $CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_$MRM\.csc 

if [ $topology == "36grid_mrm2_cnt" ]
then
    sed -i "1124s/.*/var death = $LT_PERCENT;\&\#xD;/g" $CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_$MRM\.csc 
elif [ $topology == "50random_mrm2_cnt" ]
then
    sed -i "1488s/.*/var death = $LT_PERCENT;\&\#xD;/g" $CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_$MRM\.csc 
elif [ $topology == "34cluster_mrm2_cnt" ]
then
    sed -i "1072s/.*/var death = $LT_PERCENT;\&\#xD;/g" $CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_$MRM\.csc
fi

if [ $TRAFFIC_MODEL -eq 0 ]
then
    DIR=$DATE\_$topology\_traffic$TRAFFIC_MODEL\_period$PERIOD\_LTMAX$LTMAX\_seed$SEED_NUMBER
else
    DIR=$DATE\_$topology\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_LTMAX$LTMAX\_seed$SEED_NUMBER
fi

mkdir $DIR
cd $DIR

if [ $LTMAX -eq 0 ]
then
    sed -i 's/\#define RPL_CONF_OF rpl_ltmax_of/\#define RPL_CONF_OF rpl_of0/g' $CONTIKI/platform/cooja/contiki-conf.h
else
    sed -i 's/\#define RPL_CONF_OF rpl_of0/\#define RPL_CONF_OF rpl_ltmax_of/g' $CONTIKI/platform/cooja/contiki-conf.h
fi

../param.sh $TRAFFIC_MODEL $PERIOD $ARRIVAL_RATE $LONG_WEIGHT $ETX_WEIGHT $BETA $BETA_DIV $CROSS_OPT $STROBE_CNT $CHECK $MRM $LTMAX

IN_DIR=sr\_strobe$STROBE_CNT\_B$BETA\_$BETA_DIV\_check$CHECK\_LT$LT_PERCENT
if [ ! -e $IN_DIR ]
then
    mkdir $IN_DIR
#    mkdir sr\_strobe$STROBE_CNT\_$LR_range\_$CHECK\_rou$ROUTING_NO_ENERGY
fi
cd $IN_DIR
#cd sr\_strobe$STROBE_CNT\_$LR_range\_$CHECK\_rou$ROUTING_NO_ENERGY
echo "#########################  We are in $PWD  ########################"

HERE=$PWD
cd $CONTIKI/lanada
make clean TARGET=cooja
cd $HERE

if [ $MRM -eq 0 ]
then 
    if [ ! -e COOJA.testlog ]
    then
	if [ $ONLY_LONG -eq 0 ]
	then
	    java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_$MRM\.csc -contiki="$CONTIKI"
	    #	java -mx512m -classpath $CONTIKI/tools/cooja/apps/mrm/lib/mrm.jar: -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/0729_$topology\_$LR_range\.csc -contiki="$CONTIKI"
	    # ant run_nogui -Dargs=/home/user/Desktop/Double-MAC/lanada/sim_scripts/scripts/0729_36grid_2X.csc -Ddir=$PWD
	else
	    java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_L\.csc -contiki="$CONTIKI"
	    #	ant run_nogui -Dargs=/home/user/Desktop/Double-MAC/lanada/sim_scripts/scripts/0729_36grid_2X.csc
	fi
    fi
else # If MRM mode
    if [ ! -e COOJA.testlog ]
    then
	cd $CONTIKI/tools/cooja_mrm$MRM 
	if [ $ONLY_LONG -eq 0 ]
	then
	    ant run_nogui -Dargs=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_$MRM\.csc
	else
	    ant run_nogui -Dargs=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_L\.csc
	fi
	mv build/COOJA.testlog $HERE
	cd $HERE
    fi
fi


if [ ! -e report_summary.txt ]
then
    ../../pp.sh
fi
cd ../..

echo "Simulation finished"
