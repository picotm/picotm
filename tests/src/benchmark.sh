#!/bin/bash

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

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
	          ./picotm-test $OPT -I$iterations -o$test -t$threads >> ${fullpath}.$$
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
                ./picotm-test $OPT -I$iterations -R$ccmode -o$test -t$threads >> ${fullpath}.$$
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
            ./picotm-test $OPT -I$iterations -R$ccmode -o$test -t$threads >> ${fullpath}.$$
        done

        mv ${fullpath}.$$ ${fullpath}
    done
done

#
# Create LaTEX document
#

cp benchmark/* $BASEPATH/
make -C $BASEPATH/

