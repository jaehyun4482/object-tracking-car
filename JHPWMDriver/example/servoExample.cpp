/*
 * The MIT License (MIT)

Copyright (c) 2015 Jetsonhacks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
// servoExample.cpp
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>               // 헤더 파일
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <JHPWMPCA9685.h>

#define  FIFO_FILE   "/tmp/fifo"
#define  BUFF_SIZE   1024

// Calibrated for a Robot Geek RGS-13 Servo
// Make sure these are appropriate for the servo being used!

int servoMin = 150 ;
int servoMax = 720 ;

// Map an integer from one coordinate system to another
// This is used to map the servo values to degrees
// e.g. map(90,0,180,servoMin, servoMax)
// Maps 90 degrees to the servo value

int map ( int x, int in_min, int in_max, int out_min, int out_max) {
    int toReturn =  (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min ;
    // For debugging:
    // printf("MAPPED %d to: %d\n", x, toReturn);
    return toReturn ;
}

int main() {
    int i, fd, z = 0;
    char  buff[BUFF_SIZE];
    PCA9685 *pca9685 = new PCA9685();
    if (pca9685->openPCA9685() < 0){
        printf("Error: %d", pca9685->error);
    } else {
        printf("PCA9685 Device Address: 0x%02X\n",pca9685->kI2CAddress) ;
        pca9685->setAllPWM(0,0) ;
        pca9685->reset() ;
        pca9685->setPWMFrequency(56) ;
        // 27 is the ESC key
        printf("Hit ESC key to exit\n");
        pca9685->setPWM(0,0,370); //center
        pca9685->setPWM(1,0,370); //stop

	if ( -1 == ::mkfifo( FIFO_FILE, 0666))                    // fifo 파일 생성
	{
	perror( "mkfifo() run error");
	//exit( 1);
	}

	if ( -1 == ( fd = ::open( FIFO_FILE, O_RDWR)))           //  fifo 파일 열기
	{
	perror( "open() error");
	//exit( 1);
	}

        while (true) {
	    read(fd, buff, BUFF_SIZE);
	    if(!strncmp(buff, "d1", 2))
	    {
                pca9685->setPWM(0,0,415); //right
                pca9685->setPWM(1,0,420); //low go
                z = 1;
                printf("right_1\n");
            }  
	    if(!strncmp(buff, "d2", 2))
	    {
                pca9685->setPWM(0,0,460); //right
                pca9685->setPWM(1,0,420); //low go
                z = 1;
                printf("right_2\n");
            }  
	    if(!strncmp(buff, "a1", 2))
	    {
                pca9685->setPWM(0,0,300); //left
                pca9685->setPWM(1,0,420); //low go
                z = 1;
                printf("left_1\n");
            }
	    if(!strncmp(buff, "a2", 2))
	    {
                pca9685->setPWM(0,0,230); //left
                pca9685->setPWM(1,0,420); //low go
                z = 1;
                printf("left_2\n");
            }
	    if(!strncmp(buff, "c", 1))
	    {
                pca9685->setPWM(0,0,370); //center
                z = 0;
                printf("center\n");
            } 
	    if(!strncmp(buff, "w3", 2))
	    {
                if(z == 0) {
                    pca9685->setPWM(1,0,480); //high go
                    printf("high go\n");
                }
            }
	    if(!strncmp(buff, "w2", 2))
	    {
                if(z == 0) {
                    pca9685->setPWM(1,0,440); //mid go
                    printf("mid go\n");
                }
            }
	    if(!strncmp(buff, "w1", 2))
	    {
                if(z == 0) {
                    pca9685->setPWM(1,0,410); //low go
                    printf("low go\n");
                }
            }
	    if(!strncmp(buff, "s", 2))
	    {
                pca9685->setPWM(1,0,340); //back
                printf("back\n");
            }
	    if(!strncmp(buff, "x", 2))
	    {
                pca9685->setPWM(1,0,370); //stop
                pca9685->setPWM(0,0,370); //center
                printf("stop\n");
            } 
   
            /*switch (buff) {
            case "d1":
                pca9685->setPWM(0,0,415); //right
                pca9685->setPWM(1,0,420); //low go
                z = 1;
                printf("right\n");
                break;
            case "d2":
                pca9685->setPWM(0,0,460); //right
                pca9685->setPWM(1,0,420); //low go
                z = 1;
                printf("right\n");
                break;
            case "a1":
                pca9685->setPWM(0,0,300); //left
                pca9685->setPWM(1,0,420); //low go
                printf("left\n");
                z = 1;
                break;
            case "a2":
                pca9685->setPWM(0,0,230); //left
                pca9685->setPWM(1,0,420); //low go
                printf("left\n");
                z = 1;
                break;                                
            case 'c':
                pca9685->setPWM(0,0,370); //center
                printf("center\n");
                z = 0;
                break;
            case "w3":
                if(z == 0) {
                    pca9685->setPWM(1,0,480); //high go
                    printf("mid go\n");
                }
                printf("high go\n");
                if(buff[0] == 'c')
                break;                
            case "w2":
                if(z == 0) {
                    pca9685->setPWM(1,0,440); //mid go
                    printf("mid go\n");
                }
                break;
            case "w1":
                pca9685->setPWM(1,0,410); //low go
                printf("low go\n");
                break;
            case 's':
                pca9685->setPWM(1,0,340); //back
                printf("back\n");
                break;
            case 'x':
                pca9685->setPWM(1,0,370); //stop
                pca9685->setPWM(0,0,370); //center.
                printf("stop\n");
                break;
            case 'q':
                pca9685->setPWM(0,0,370); //center.
                pca9685->setPWM(1,0,370); //stop.
                pca9685->closePCA9685();
                printf("center\n");
                printf("stop\n");
                return 0;
                break;
            default:
                break;
            }*/
        }
    }
    pca9685->closePCA9685();
}

//pca9685->setPWM(0,0,438); //right
//pca9685->setPWM(0,0,324); //center
//pca9685->setPWM(0,0,229); //left
