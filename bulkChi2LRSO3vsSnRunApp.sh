#!/bin/bash
# lat: [-90,90]
# lon: [-180,180]
# command option 1: events to cutoff
# how to run it: bulkChi2LRSO3vsSnRunApp.sh <nEveOffSet> <grid presicion i.e 10 or 5 or 2> <alpha>
 
if [ $# -ne 3 ];
then
    echo "Please: "
    echo "./bulkChi2LRSO3vsSnRunApp.sh <nEveOffSet> <grid presicion i.e 10 or 5 or 2> <alpha>"
    exit 1
fi

l="LAT"
ll="LON"
latmin=-90
latmax=90
lonmin=-180
lonmax=180
#grid presicion
h=$2
alpha=$3

for (( i=$lonmin; i <= $lonmax; i=i+h ))
do
    for (( j=$latmin; j <= $latmax; j=j+h ))
    do
	echo "== START ./chi2LRSO3vsSnRunApp $1 $l$j$ll$i ==="
	#sleep 1
	./chi2LRSO3vsSnRunApp -E$1 -N$l$j$ll$i -I$alpha
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "
	echo "============================================== END $l$j$ll$i == "

	echo " "
    done
done
