#!/bin/bash
if [ ! -e example ];then
    make
fi

sudo chmod 666 /dev/i2c-1
./example
