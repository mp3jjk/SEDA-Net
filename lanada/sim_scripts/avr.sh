#!/bin/bash 

MODE=$1

if [ $MODE == "lifetime" ]
then
	./lifetime.sh > temp.txt
elif [ $MODE == "delay" ]
then
	./delay.sh > temp.txt
elif [ $MODE == "prr" ]
then
	./prr.sh > temp.txt
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
			count=-1
			continue
		fi

		if [ $count -ge 0 ]
		then
			add=`echo $line | cut -d ' ' -f4`
			base=`echo $line | cut -d ' ' -f1`
			y[$count]=$base
			let " x[$count] = ${x[$count]}+$add " 
			# echo "$count-th variable is ${x[$count]}"
			let "count=$count+1"
		fi
	done < temp.txt
	count=0
	echo "################ $category ################"
	while [ ${x[$count]} -gt 0 ]
	do
		let "x[$count]=${x[$count]}/$cat_count"
		echo "${y[$count]} average: ${x[$count]}"
		let "count=$count+1"
	done
done < temp3.txt

rm temp*.txt
