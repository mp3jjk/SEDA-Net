#!/bin/bash

CONTIKI=/media/user/Harddisk/Double-MAC

#For poisson traffic
sed -i 's/\#define TRAFFIC_MODEL 0/\#define TRAFFIC_MODEL 1/g' $CONTIKI/lanada/param.h

for arrival in 100
do
mkdir 0515_2_simulation_poisson$arrival
cd 0515_2_simulation_poisson$arrival

#mkdir debug_LSA-MAC
#cd debug_LSA-MAC

	for weight in 2
	do
	    #for topology in 16linear 36grid 50random
	    for topology in 50random
		do
			for LR_range in 2X
			do
				for energy in 200 
				do
					for LS in LS S
					do
						if [ ! -e $topology\_E$energy\_LR$LR_range_\$LS\_W$weight ]
						then
							mkdir $topology\_E$energy\_LR$LR_range\_$LS\_W$weight
						fi

						case "$weight" in 
							2 )	sed -i 's/\#define LONG_WEIGHT_RATIO 5/\#define LONG_WEIGHT_RATIO 2/g' $CONTIKI/lanada/param.h;;
							5 )	sed -i 's/\#define LONG_WEIGHT_RATIO 2/\#define LONG_WEIGHT_RATIO 5/g' $CONTIKI/lanada/param.h;;
						esac
						
						case "$LS" in
							S  )  sed -i 's/\#define DUAL_RADIO 1/\#define DUAL_RADIO 0/g' $CONTIKI/platform/cooja/contiki-conf.h;;
							LS  ) sed -i 's/\#define DUAL_RADIO 0/\#define DUAL_RADIO 1/g' $CONTIKI/platform/cooja/contiki-conf.h;;
						esac

						case "$arrival" in
							100  )  sed -i 's/\#define ARRIVAL_RATE 100/\#define ARRIVAL_RATE 200/g' $CONTIKI/lanada/param.h;;
							200  ) sed -i 's/\#define ARRIVAL_RATE 200/\#define ARRIVAL_RATE 100/g' $CONTIKI/lanada/param.h;;
						esac

						cd  $topology\_E$energy\_LR$LR_range\_$LS\_W$weight
						echo "#########################  We are in $PWD  ########################"
						if [ ! -e COOJA.testlog ]
						then
							java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/0502_$topology\_$LR_range\.csc -contiki="$CONTIKI"
						fi
						../../pp.sh
						cd ..
					done
				done
			done
		done
	done
	cd ..
done

echo "Simulation finished"
