#!/bin/bash

set -e

if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

cd `pwd`/build
rm -rf *
cmake .. && make 

if [ ! -d /usr/include/muduo_study ]; then
    mkdir /usr/include/muduo_study
fi

cd ..
cd `pwd`/src

for head in `ls *.h`
do
    cp $head /usr/include/muduo_study
done

cd ..
cp `pwd`/lib/libmuduo_study.so /usr/lib

