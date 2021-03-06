#!/bin/bash

JOONKI=0

if [ $JOONKI -eq 0 ]
then
    CONTIKI=/media/user/Harddisk/Developing-Dual-Net
else
    CONTIKI=~/Developing-Dual-Net
fi

echo "Long range simulation"
sed -i 's/\#define DUAL_RADIO 0/\#define DUAL_RADIO 1/g' $CONTIKI/platform/cooja/contiki-conf.h
sed -i 's/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 1/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 0/g' $CONTIKI/platform/cooja/contiki-conf.h

topology=$1

TRAFFIC_MODEL=$2
PERIOD=$3
ARRIVAL_RATE=$4

LR_range=$5
LONG_WEIGHT=$6
ETX_WEIGHT=$7
BETA=$8
BETA_DIV=$9
CROSS_OPT=${10}

STROBE_CNT=${11}
CHECK=${12}

DATE=${13}

ONLY_LONG=${14}
WAKE_UP=${15}
SEED_NUMBER=${16}
MRM=${17}
LT_PERCENT=${18}
LTMAX=${19}

if [ $MRM -eq 0 ]
then
    if [ $WAKE_UP -eq 0 ]
    then
	sed -i "11s/.*/    <randomseed>$SEED_NUMBER<\/randomseed>/" $CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\.csc 
    else
	sed -i "11s/.*/    <randomseed>$SEED_NUMBER<\/randomseed>/" $CONTIKI/lanada/sim_scripts/scripts/$topology\_W\_$LR_range\.csc 
    fi

else
    if [ $WAKE_UP -eq 0 ]
    then
	sed -i "11s/.*/    <randomseed>$SEED_NUMBER<\/randomseed>/" $CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_$MRM\.csc 
    else
	sed -i "11s/.*/    <randomseed>$SEED_NUMBER<\/randomseed>/" $CONTIKI/lanada/sim_scripts/scripts/$topology\_W\_$LR_range\_$MRM\.csc 
    fi

    if [ $topology == "36grid_mrm2_cnt" ]
    then
	sed -i "1124s/.*/var death = $LT_PERCENT;\&\#xD;/g" $CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_$MRM\.csc 
	sed -i "1124s/.*/var death = $LT_PERCENT;\&\#xD;/g" $CONTIKI/lanada/sim_scripts/scripts/$topology\_W\_$LR_range\_$MRM\.csc 
    elif [ $topology == "50random_mrm2_cnt" ]
    then
	sed -i "1488s/.*/var death = $LT_PERCENT;\&\#xD;/g" $CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_$MRM\.csc 
	sed -i "1488s/.*/var death = $LT_PERCENT;\&\#xD;/g" $CONTIKI/lanada/sim_scripts/scripts/$topology\_W\_$LR_range\_$MRM\.csc 
    elif [ $topology == "34cluster_mrm2_cnt" ]
    then
	sed -i "1072s/.*/var death = $LT_PERCENT;\&\#xD;/g" $CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_$MRM\.csc
	sed -i "1072s/.*/var death = $LT_PERCENT;\&\#xD;/g" $CONTIKI/lanada/sim_scripts/scripts/$topology\_W\_$LR_range\_$MRM\.csc
    fi
fi

if [ $TRAFFIC_MODEL -eq 0 ]
then
    DIR=$DATE\_$topology\_traffic$TRAFFIC_MODEL\_period$PERIOD\_LTMAX$LTMAX\_seed$SEED_NUMBER
else
    DIR=$DATE\_$topology\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_LTMAX$LTMAX\_seed$SEED_NUMBER
fi

mkdir $DIR
cd $DIR

if [ $ONLY_LONG -eq 0 ]
then
    sed -i 's/\#define ONLY_LONG 1/\#define ONLY_LONG 0/g' $CONTIKI/platform/cooja/contiki-conf.h
else
    sed -i 's/\#define ONLY_LONG 0/\#define ONLY_LONG 1/g' $CONTIKI/platform/cooja/contiki-conf.h
fi

if [ $WAKE_UP -eq 0 ]
then
    sed -i 's/\#define WAKEUP_RADIO 1/\#define WAKEUP_RADIO 0/g' $CONTIKI/platform/cooja/contiki-conf.h
else
    sed -i 's/\#define WAKEUP_RADIO 0/\#define WAKEUP_RADIO 1/g' $CONTIKI/platform/cooja/contiki-conf.h
fi

if [ $LTMAX -eq 0 ]
then
    sed -i 's/\#define RPL_CONF_OF rpl_ltmax_of/\#define RPL_CONF_OF rpl_of0/g' $CONTIKI/platform/cooja/contiki-conf.h
else
    sed -i 's/\#define RPL_CONF_OF rpl_of0/\#define RPL_CONF_OF rpl_ltmax_of/g' $CONTIKI/platform/cooja/contiki-conf.h
fi

../param.sh $TRAFFIC_MODEL $PERIOD $ARRIVAL_RATE $LONG_WEIGHT $ETX_WEIGHT $BETA $BETA_DIV $CROSS_OPT $STROBE_CNT $CHECK $MRM $LTMAX

IN_DIR=lr\_weight$LONG_WEIGHT\_LR_range$LR_range\_L$ONLY_LONG\_WAKE$WAKE_UP\_B$BETA\_$BETA_DIV\_check$CHECK\_strobe$STROBE_CNT\_LT$LT_PERCENT
if [ ! -e $IN_DIR ]
then
    mkdir $IN_DIR
fi
cd $IN_DIR

echo "#########################  We are in $PWD  ########################"

HERE=$PWD
if [ $MRM -eq 0 ]
then
    cd $CONTIKI/lanada
else
    cd $CONTIKI/lanada_$MRM
fi
make clean TARGET=cooja
cd $HERE

if [ $MRM -eq 0 ]
then
	if [ ! -e COOJA.testlog ]
	then
	#	cd $CONTIKI/tools/cooja
			 java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\.csc -contiki="$CONTIKI"
	#		ant run_nogui -Dargs=/home/user/Desktop/Double-MAC/lanada/sim_scripts/scripts/0729_36grid_2X.csc
	#	cd $HERE
	fi
else
	if [ ! -e COOJA.testlog ]
	then
		cd $CONTIKI/tools/cooja_mrm$MRM
		if [ $WAKE_UP -eq 0 ]
		then
		    ant run_nogui -Dargs=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\_$MRM\.csc
		else
		    ant run_nogui -Dargs=$CONTIKI/lanada/sim_scripts/scripts/$topology\_W\_$LR_range\_$MRM\.csc
		fi
		mv build/COOJA.testlog $HERE
		cd $HERE
	fi
fi

# if [ ! -e report_summary.txt ]
# then
# #    ../../pp.sh
# fi
cd ../..

echo "Simulation finished"
