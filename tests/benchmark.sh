#!/bin/bash

declare -r OPT="-N -btime -c60000 -n1 -v1"
declare -r BASEPATH="benchmark.`date -u +%F-%R`"
declare -r FILENAME="test-benchmark"
declare -r EXT="dat"

declare -r ncores=`cat /proc/cpuinfo | grep processor | wc -l`

mkdir $BASEPATH

#
# Allocator tests
#

for test in 6 7 68
do
    for iterations in 10 100 1000
    do
        declare fullpath="$BASEPATH/$FILENAME`echo "$OPT" | sed s/\ //g`-o${test}-I${iterations}.$EXT"

        touch ${fullpath}.$$

  	    for threads in `seq $ncores`
	      do
	          ./tlctest $OPT -I$iterations -o$test -t$threads >> ${fullpath}.$$
  	    done

  	    mv ${fullpath}.$$ ${fullpath}
	done
done

#
# File-I/O tests
#

# transactional I/O

for test in 28 30 32 34 36 38 40 42 44 46 48 50 52 54 56 58
do
    for iterations in 1 10 100
    do
        for ccmode in noundo 2pl ts
        do 
            declare fullpath="$BASEPATH/$FILENAME`echo "$OPT" | sed s/\ //g`-o${test}-I${iterations}-R${ccmode}.$EXT"

            touch ${fullpath}.$$

            for threads in `seq $ncores`
            do
                ./tlctest $OPT -I$iterations -R$ccmode -o$test -t$threads >> ${fullpath}.$$
            done

            mv ${fullpath}.$$ ${fullpath}
        done
    done
done

# non-transactional I/O

for test in 29 31 33 35 37 39 41 43 45 47 49 51 53 55 57 59
do
    for iterations in 1 10 100
    do
        declare fullpath="$BASEPATH/$FILENAME`echo "$OPT" | sed s/\ //g`-o${test}-I${iterations}.$EXT"

        touch ${fullpath}.$$

        for threads in `seq $ncores`
        do
            ./tlctest $OPT -I$iterations -R$ccmode -o$test -t$threads >> ${fullpath}.$$
        done

        mv ${fullpath}.$$ ${fullpath}
    done
done

#
# Create LaTEX document
#

cp benchmark/* $BASEPATH/
make -C $BASEPATH/

