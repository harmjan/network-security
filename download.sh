#!/bin/bash

# Download all the data on 31 July 2013, there should
# be an attack announced on 07:36:36 according to
# http://research.dyn.com/2013/11/mitm-internet-hijacking/#!prettyPhoto

# Url's from ripe are formatted like:
# http://data.ris.ripe.net/rrc00/2015.03/updates.20150316.2350.gz
# http://data.ris.ripe.net/rrc00/yyyy.mm/updates.yyyymmdd.hhmm.gz
# There are package for every 5 minutes

# The collector to download from, the list of what collecter corresponds
# to where is on https://www.ripe.net/data-tools/stats/ris/ris-raw-data
for COLLECTOR in `seq 0 1 16`
do
	# The date data that is still static, by using variables
	# is it easy to change and can it be iterated over if we
	# want to download for example an entire year.
	YEAR=2013
	MONTH=7
	DAY=31

	# Loop over all the 5 minutes sequences in the range we care
	# about and download and extract the files.
	for HOUR in `seq 0 1 23`
	do
		for MINUTE in `seq 0 5 55`
		do
			# Create the filename
			URL=data.ris.ripe.net
			DIRECTORY=`printf "/rrc%0*d/%0*d.%0*d/" 2 $COLLECTOR 4 $YEAR 2 $MONTH`
			FILENAME=`printf "updates.%0*d%0*d%0*d.%0*d%0*d.gz" 4 $YEAR 2 $MONTH 2 $DAY 2 $HOUR 2 $MINUTE`
			#echo ${URL}${DIRECTORY}${FILENAME}
			wget -q ${URL}${DIRECTORY}${FILENAME}
			NEW_NAME=`printf "%0*d.%0*d-%0*d-%0*d.%0*d:%0*d" 2 $COLLECTOR 4 $YEAR 2 $MONTH 2 $DAY 2 $HOUR 2 $MINUTE`
			mv $FILENAME data/${NEW_NAME}.gz
			gunzip data/${NEW_NAME}.gz
		done
	done
done
