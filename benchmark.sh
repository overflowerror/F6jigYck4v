#!/bin/bash

make benchmark
if test $? != 0; then
	exit 1
fi

export LD_LIBRARY_PATH=.

iterations=$1

if test "$iterations" = ""; then
	iterations=200
fi

resultfile=/dev/shm/$$.result
timefile=/dev/shm/$$.time

encoding=()
decoding=()

minMax() {
	tmp=$(sort -g)
	echo -n "min:   "
	echo "$tmp" | head -n 1
	echo -n "max:   "
	echo "$tmp" | tail -n 1
}

standardDeviation() {
	awk '{ sum += $1; sumsq += $1 * $1 } END { print "mu:    " sum/NR "\nsigma: " sqrt(sumsq/NR - (sum/NR)**2)}'
}

for i in $(seq 0 $iterations); do
	echo -n "."
	parameter=$RANDOM
	./benchmark $2 $parameter 1> $resultfile 2> $timefile 
	result=$(cat $resultfile)
	ns=$(cat $timefile | awk -F: '{ print $2 }' | awk '{ print $1 }')

	encoding+=($ns)

	./benchmark $2 -d $result 1> $resultfile 2> $timefile
	result=$(cat $resultfile)
	ns=$(cat $timefile | awk -F: '{ print $2 }' | awk '{ print $1 }')

	decoding+=($ns)

	if test "$result" != "$parameter"; then
		echo
		echo "Error on input $parameter. Result: $result"
	fi
done

echo
echo
echo "Encoding: "
printf "%s\n" "${encoding[@]}" | minMax
printf "%s\n" "${encoding[@]}" | standardDeviation
echo
echo "Decoding: "
printf "%s\n" "${decoding[@]}" | minMax
printf "%s\n" "${decoding[@]}" | standardDeviation

rm benchmark
