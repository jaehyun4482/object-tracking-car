#!/bin/bash

if [ ! -e yolov3-tiny.weights ];then
    wget https://pjreddie.com/media/files/yolov3-tiny.weights
fi

if [ ! -e darknet ];then
    make -j6
fi
./darknet detector demo cfg/coco.data cfg/yolov3-tiny.cfg yolov3-tiny.weights "'nvarguscamerasrc ! video/x-raw(memory:NVMM), width=1920, height=1080, format=(string)NV12, framerate=(fraction)30/1 ! nvtee ! nvvidconv flip-method=0 ! video/x-raw, width=(int)1280, height=(int)720, format=(string)BGRx ! videoconvert ! appsink'"
