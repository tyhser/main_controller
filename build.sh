#!/bin/bash

make project_clen

make all -j8
make flash
a=$?
b=0

if [ $a -ne 0 ]
then
    make flash
else
    echo "build done"
fi

