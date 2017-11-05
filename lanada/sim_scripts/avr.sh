#!/bin/bash 

MODE=$1
ARG=$2

if [ $MODE == "lifetime" ]
then
	./lifetime.sh $ARG > temp.txt
elif [ $MODE == "delay" ]
then
	./delay.sh $ARG > temp.txt
elif [ $MODE == "prr" ]
then
	./prr.sh $ARG > temp.txt
fi

grep "#" temp.txt | awk '{print $2}' > temp2.txt
awk -F 'seed' '{print $1}' temp2.txt | awk '!x[$0]++' > temp3.txt 
# sed 's/_//g' temp.txt | sed '/#/d' > temp4.txt
while read category
do
	cat_count=0;
	while read line 
	do
		if [[ $line =~ "$category" ]]
		then
			count=0
			let "cat_count=$cat_count+1"
			continue
		elif [ -z "$line" ]
		then
			echo
			max_count=$count
			count=-1
			continue
		fi

		if [ $count -ge 0 ]
		then
			add=`echo $line | cut -d ' ' -f4`
			if [ $MODE == "lifetime" ]
			then
				base=`echo $line | cut -d ' ' -f1`
			else
				base=`echo $line | cut -d ' ' -f2`
			fi
			y[$count]=$base
			if [ $MODE == "prr" ]
			then
				if [ -z ${x[$count]} ]
				then
					x[$count]=0
				fi
				x[$count]=`echo "scale=3;${x[$count]}+$add"|bc`
			else
				let " x[$count] = ${x[$count]}+$add " 
			fi
			# echo "$count-th variable is ${x[$count]}"
			let "count=$count+1"
		fi
	done < temp.txt
	count=0
	echo "################ $category ################"
	while [ $count -lt $max_count ]
	do
		if [ $MODE == "prr" ]
		then
			x[$count]=`echo "scale=3;${x[$count]}/$cat_count"|bc`
		else
			let "x[$count]=${x[$count]}/$cat_count"
		fi
		echo "${y[$count]} average: ${x[$count]}"
		let "count=$count+1"
	done
done < temp3.txt

rm temp*.txt
