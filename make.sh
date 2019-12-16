cd ./darknet
chmod +x start_yolo.sh
make clean
make -j16
cd ../JHPWMDriver/example
chmod +x start_servo.sh
make clean
make
cd ../../JHVL53L0X/example
chmod +x start_range.sh
make clean
make
cd ../../