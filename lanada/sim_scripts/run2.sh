#!/bin/bash

SR=0 # Decide whether SR simulation runs or not
LR=1 # For LR case
ONLY_LONG=0 # SR = 1 with only Long
WAKE_UP=1 # LR = 1 with Wake-up radio
TRAFFIC=1 # 0 = periodic, 1 = poisson
VAR_PERIOD=(15)
VAR_ARRIVAL=(30)
VAR_TOPOLOGY=("50random_mrm2_cnt")
VAR_LR_RANGE=("2X")

VAR_LR_WEIGHT=(1)
VAR_ETX_WEIGHT=1
VAR_BETA=(1)
VAR_BETA_DIV=(1)
VAR_CROSS_OPT=0

VAR_STROBE_CNT=0
VAR_CHECK_RATE=(8)

DATE="G2"
SEED_NUMBER=("1" "2" "3" "4" "5")
MRM=2
VAR_PERCENT=("1")

# SR_RANGE simulation

if [ $SR -eq 1 ]
then
    if [ $TRAFFIC -eq 0 ]
    then
	for seed in "${SEED_NUMBER[@]}"
	do
	    for period in "${VAR_PERIOD[@]}"
	    do
		for topology in "${VAR_TOPOLOGY[@]}"
		do
		    for range in "${VAR_LR_RANGE[@]}"
		    do
			for beta in "${VAR_BETA[@]}"
			do
			    for beta_div in "${VAR_BETA_DIV[@]}"
			    do
				for percent in "${VAR_PERCENT[@]}"
				do
				    for check in "${VAR_CHECK_RATE[@]}"
				    do
					./sr_run.sh $topology $TRAFFIC $period 0 $VAR_ETX_WEIGHT $beta $beta_div $VAR_CROSS_OPT $VAR_STROBE_CNT $check "${DATE}" $seed $MRM $percent
				    done
				done
			    done
			done
		    done
		done
	    done
	done
    else
	for seed in "${SEED_NUMBER[@]}"
	do
	    for arrival in "${VAR_ARRIVAL[@]}"
	    do
		for topology in "${VAR_TOPOLOGY[@]}"
		do
		    for range in "${VAR_LR_RANGE[@]}"
		    do
			for beta in "${VAR_BETA[@]}"
			do
			    for beta_div in "${VAR_BETA_DIV[@]}"
			    do
				for percent in "${VAR_PERCENT[@]}"
				do
				    for check in "${VAR_CHECK_RATE[@]}"
				    do
					./sr_run.sh $topology $TRAFFIC 0 $arrival $VAR_ETX_WEIGHT $beta $beta_div $VAR_CROSS_OPT $VAR_STROBE_CNT $check "${DATE}" $seed $MRM $percent
				    done
				done
			    done
			done
		    done
		done
	    done
	done
    fi
fi

# LR_RANGE simulation
if [ $LR -eq 1 ]
then
    if [ $TRAFFIC -eq 0 ]
    then
	for seed in "${SEED_NUMBER[@]}"
	do
	    for period in "${VAR_PERIOD[@]}"
	    do
		for topology in "${VAR_TOPOLOGY[@]}"
		do
		    for range in "${VAR_LR_RANGE[@]}"
		    do
			for weight in "${VAR_LR_WEIGHT[@]}"
			do
			    for beta in "${VAR_BETA[@]}"
			    do
				for beta_div in "${VAR_BETA_DIV[@]}"
				do
				    for percent in "${VAR_PERCENT[@]}"
				    do
					for check in "${VAR_CHECK_RATE[@]}"
					do
					    ./lr_run.sh $topology $TRAFFIC $period 0 $range $weight $VAR_ETX_WEIGHT $beta $beta_div $VAR_CROSS_OPT $VAR_STROBE_CNT $check "${DATE}" $ONLY_LONG $WAKE_UP $seed $MRM $percent
					done
				    done
				done
			    done
			done
		    done
		done
	    done
	done
    else
	for seed in "${SEED_NUMBER[@]}"
	do
	    for arrival in "${VAR_ARRIVAL[@]}"
	    do
		for topology in "${VAR_TOPOLOGY[@]}"
		do
		    for range in "${VAR_LR_RANGE[@]}"
		    do
			for weight in "${VAR_LR_WEIGHT[@]}"
			do
			    for beta in "${VAR_BETA[@]}"
			    do
				for beta_div in "${VAR_BETA_DIV[@]}"
				do
				    for percent in "${VAR_PERCENT[@]}"
				    do
					for check in "${VAR_CHECK_RATE[@]}"
					do
					    ./lr_run.sh $topology $TRAFFIC 0 $arrival $range $weight $VAR_ETX_WEIGHT $beta $beta_div $VAR_CROSS_OPT $VAR_STROBE_CNT $check "${DATE}" $ONLY_LONG $WAKE_UP $seed $MRM $percent
					done
				    done
				done
			    done
			done
		    done
		done
	    done
	done
    fi
fi
