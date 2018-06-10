#!/bin/bash

#TOPOLOGY=("36grid" "50random")
#TOPOLOGY=("xxx")


#for topo in "${TOPOLOGY[@]}" do
#if [ -d $dir -a -n `echo "$dir" | grep $topo` ]
#then
for dir in */ ; do
    cd $dir
    for indir in */ ; do
	cd $indir
	seed=`expr substr $dir 41 5`
	name=`expr substr $indir 1 30`
	sr=`expr substr $indir 1 2`
	if [ $sr = "sr" ]
	then
	    cat COOJA.testlog | grep "RESULT" > ../com_$sr\_$seed
	else
	    cat COOJA.testlog | grep "RESULT" > ../com_$name\_$seed
	fi
    # if [ ! -e report_summary.txt ]
    # then
    #     ../../pp_test.sh
    # fi
	cd ..
    done
    cd ..
done
#done
