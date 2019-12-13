all:
	g++ servoExample.cpp ../src/JHPWMPCA9685.cpp -I../src -li2c -o servoExample
	gcc -o ex_fifo_s ex_fifo_s.c
clean:
	rm -rf ./servoExample ./ex_fifo_s
