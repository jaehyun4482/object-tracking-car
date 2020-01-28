#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <VL53L0X.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define  FIFO_FILE     "/tmp/VL53L0X"
#define  BUFF_SIZE   1024

int from_VL53L0X;
char buff[BUFF_SIZE];

int getkey() {
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}

int main() {

    VL53L0X *sensor = new VL53L0X();
    // Open the sensor
    if (!sensor->openVL53L0X()) {
        // Trouble
        printf("Unable to open VL53L0X") ;
        exit(-1) ;
    }
    sensor->init();
    sensor->setTimeout(500);
    // 27 is the ESC key
    printf("Example for VL53L0X\n");
    printf("Place object in front of sensor for reading ...\n") ;
    printf("Press the ESC key to stop the program\n");
    if (-1 == (from_VL53L0X = open(FIFO_FILE, O_RDWR)))
    {
        if ( -1 == mkfifo( FIFO_FILE, 0666))
        {
            perror( "mkfifo() run error");
            exit( 1);
        }
        if ( -1 == ( from_VL53L0X = open( FIFO_FILE, O_RDWR)))
        {
            perror( "open() error");
            exit( 1);
        }
    }
/*    while(getkey() != 27){
        int distance = sensor->readRangeSingleMillimeters();
        if (sensor->timeoutOccurred()) {
            printf("Sensor timeout!\n");
        } else {
            // If distance > 2000, no return received; Don't print it
            if (distance < 100) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '0';
                write(from_VL53L0X, buff, 1 );
            } else if (distance < 300) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '1';
                write(from_VL53L0X, buff, 1 );
            } else if (distance < 500) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '2';
                write(from_VL53L0X, buff, 1 );
            } else if (distance < 700) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '3';
                write(from_VL53L0X, buff, 1 );
            } else if (distance < 900) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '4';
                write(from_VL53L0X, buff, 1 );
            } else if (distance < 1100) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '5';
                write(from_VL53L0X, buff, 1 );
            } else {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '6';
                write(from_VL53L0X, buff, 1 );
            }
        }
    }*/
    while(getkey() != 27){
        int distance = sensor->readRangeSingleMillimeters();
        if (sensor->timeoutOccurred()) {
            printf("Sensor timeout!\n");
        } else {
            // If distance > 2000, no return received; Don't print it
            if (distance < 100) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '0';
                write(from_VL53L0X, buff, 1 );
            } else if (distance > 150 && distance <= 300) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '1';
                write(from_VL53L0X, buff, 1 );
            } else if (distance > 300 && distance <= 450) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '2';
                write(from_VL53L0X, buff, 1 );
            } else if (distance > 450 && distance <= 600) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '3';
                write(from_VL53L0X, buff, 1 );
            } else if (distance > 600 && distance <= 750) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '4';
                write(from_VL53L0X, buff, 1 );
            } else if (distance > 750 && distance <= 850) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '5';
                write(from_VL53L0X, buff, 1 );
            } else if (distance > 850 && distance <= 1000) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '6';
                write(from_VL53L0X, buff, 1 );
            } else if (distance > 1000 && distance <= 1100) {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '7';
                write(from_VL53L0X, buff, 1 );
            } else {
                printf("\nDistance: %5d mm ",distance);
                buff[0] = '8';
                write(from_VL53L0X, buff, 1 );
            }
        }
    }
    printf("\n\n") ;
    sensor->closeVL53L0X();
}
