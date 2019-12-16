#!/bin/bash

if [ ! -e yolov3-tiny.weights ];then
    wget https://pjreddie.com/media/files/yolov3-tiny.weights
fi

if [ ! -e darknet ];then
    make -j8
fi
./darknet detector demo cfg/coco.data cfg/yolov3-tiny.cfg yolov3-tiny.weights -c 0 -thresh 0.3
