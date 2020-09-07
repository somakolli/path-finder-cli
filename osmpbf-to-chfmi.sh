#!/bin/bash

INPUT=$1
INTERMEDIATE=$2
OUTPUT=$3
if [ -z "$INPUT" ]
then
	echo "no input file"
	exit 1
fi
if [ -z "$OUTPUT" ]
then
	echo "no output file"
	exit 1
fi
if [ -z "$INTERMEDIATE" ]
then
	echo "no intermediate file"
	exit 1
fi

./OsmGraphCreator/build/creator/creator -t distance -g fmimaxspeedtext -o $INTERMEDIATE -c OsmGraphCreator/data/configs/car.cfg $INPUT
./ch_constructor/build/ch_constructor -f FMI -i $INTERMEDIATE -o $OUTPUT -t 20
