# BGP Historical data analyzer
The goal of this program is to detect BGP hijacking attacks from historical BGP data. The data from https://www.ripe.net/data-tools/stats/ris/ris-raw-data in the MRT format is used.

This is a project I do solo for the ET4397IN Network Security course. The software quality and commit behaviour might at times be questionable.

# Building the software
The software is build using [CMake](http://www.cmake.org/), you can build the software with:
```
mkdir build
cd build
cmake ..
make
```

# Running the software
Download the data files of the ranges you want to analyze into a data/ directory with the download.sh script. The data directory needs to be created before running the script, in my experiment was 1 day of data from all collectors about 2.4 gigabytes of data.

The route announcements are now stored per collector chronologically. The bgp-extract program rewrites the announcement to a file per prefix ordered chronologically. Create a folder ip/ and run the program with:
```
time ./bgp-extract data/*
```
In my case did this take about 5 hours for a day of announcements and generated about 505000 files with a combined size of 5 Gigabytes. Not all MRT and BGP types are supported, but if the program doesn't know how to handle them it prints an warning message and continues. The program gives an indication of what it is doing and how far into the MRT files it is.
```
Reading file 362 of 3456 : data/01.2013-07-31.06:05
        Unsupported MRT type 4 with subtype 0 message ignored
Reading file 3456 of 3456 : data/15.2013-07-31.23:55 | Writing 68000/68805

real    303m28.701s
user    44m20.292s
sys     21m28.938s
```

After the extracting you can analyze the data with bgp-analyze. Giving all the filenames via * expansion doesn't work anymore so the list of ip prefixes is generated with ls. The actual output is via the error stream, the command below will do an analysis and create a file warnings with the points of interest. The analysis takes about 1 hour for a day of announcements.
```
ls ip/ > ips
time ./bgp-analyze < ips 2> warnings
```

```
Reading in filenames
Sorting filenames
Doing analysis
504990/504990
All done!

real    56m26.965s
user    2m25.610s
sys     0m40.907s
```
