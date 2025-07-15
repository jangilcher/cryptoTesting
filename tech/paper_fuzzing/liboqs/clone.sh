#!/bin/bash

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    exit 1
fi

ALG=$1
rm -rf $ALG
mkdir -p $ALG
cp * $ALG
sed -i -e 's/include "/include "..\//g' $ALG/*.c
sed -i -e 's/ROOTDIR=/ROOTDIR=..\//g' $ALG/Makefile
sed -i -e 's/BASEDIR=/BASEDIR=..\//g' $ALG/Makefile
sed -i -e 's/-include[[:space:]]/-include ..\//g' $ALG/Makefile
