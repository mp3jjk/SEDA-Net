#!/bin/bash


CONTIKI=~/Desktop/Double-MAC

#For periodic traffic
sed -i 's/\#define TRAFFIC_MODEL 1/\#define TRAFFIC_MODEL 0/g' $CONTIKI/lanada/param.h

for period in 60
do
	
mkdir 0516_v2_simulation_period$period
cd 0516_v2_simulation_period$period

#mkdir debug
#cd debug

	for weight in 2
	do
		#for topology in 16linear 36grid 50random
		for topology in 50random
		do
			for LR_range in 2X
			do
				for energy in 200 
				do
					for LS in LS
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

						case "$period" in
							30  )  sed -i 's/\#define PERIOD 60/\#define PERIOD 30/g' $CONTIKI/lanada/param.h;;
							60  ) sed -i 's/\#define PERIOD 30/\#define PERIOD 60/g' $CONTIKI/lanada/param.h;;
						esac

						cd  $topology\_E$energy\_LR$LR_range\_$LS\_W$weight
						echo "#########################  We are in $PWD  ########################"
						if [ ! -e COOJA.testlog ]
						then
<<<<<<< HEAD
							java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/0502_$topology\_$LR_range\.csc -contiki="$CONTIKI"
							#java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/0502_$topology\_$LR_range\_seed123457.csc -contiki="$CONTIKI"
=======
							java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/0502_$topology\_$LR_range\.csc -contiki="$CONTIKI"
>>>>>>> f5d41c35c0bf95750a2284a90b750d3af89d1c64
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
