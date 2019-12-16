#!/bin/bash
if [ ! -e example ];then
    make
fi

sudo chmod 666 /dev/i2c-8
./servoExample
