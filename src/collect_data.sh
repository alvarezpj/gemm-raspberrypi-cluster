#!/bin/bash

# collect sample data


file=$1  # source filename
size=512 # matrix size

# produce executable name 
temp=`echo -n $file | wc -c`
executable=`echo -n $file | head -c $(($temp-2))`

parallel-ssh mkdir -p ~/gemm-raspberrypi-cluster/src/
echo

# compile and run program for 3 different sizes
for i in {1..3}
do
    size=$(($size*2))
    sed -i "s/#define LENGTH.*/#define LENGTH        $size/" ./$file
    make -s $executable
    parallel-scp ./$executable ~/gemm-raspberrypi-cluster/src/  
    echo

    echo results for $file \($size\x$size\)

    for j in {1..10}
    do
        echo -n $j\)" " 
        #./$executable
        mpirun ~/gemm-raspberrypi-cluster/src/$executable
        sleep 60
    done

    echo
done

# clean up
make -s clean
