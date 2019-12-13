
/*
    C socket server example
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write

#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>

// for yolo
#include <sys/stat.h>
#define  FIFO_FROM_YOLO   "/tmp/from_yolo_fifo"
#define  FIFO_TO_YOLO     "/tmp/to_yolo_fifo"
#define  BUFF_SIZE   1024

int   counter = 0;
int   fd_from_yolo;
int   fd_to_yolo;
char  buff[BUFF_SIZE];

int     handle;
struct  termios  oldtio,newtio;
    
char    *TitleMessage = "Welcome Serial Port\r\n";
char    Buff[256];
int     RxCount;
int     loop;
int     ending;    
int     key;
            
// for select
int     i;
int     n;
int     state;

// 쓰레드 함수
void *t_function(void *data)
{
    int id;
    int i = 0;
    id = *((int *)data);

    //from yolo
    while(1)
    {
        while(read(fd_from_yolo, buff, BUFF_SIZE) != NULL)
        {
            write( handle, buff, 1 );
            printf("%s", buff);
            memset(buff, 0x00, BUFF_SIZE);
        }
    }
}

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
    fd_set readfds;
    pthread_t p_thread[2];
    int thr_id;
    int status;
    int a = 1;
    int b = 2;

    // 쓰레드 생성 아규먼트로 1 을 넘긴다.  
    thr_id = pthread_create(&p_thread[0], NULL, t_function, (void *)&a);
    if (thr_id < 0)
    {
        perror("thread create error : ");
        exit(0);
    }
    //for yolo 
    if ( -1 == ( fd_from_yolo = open( FIFO_FROM_YOLO, O_RDWR)))
    {
        if ( -1 == mkfifo( FIFO_FROM_YOLO, 0666))
        {
            perror( "mkfifo() run error");
            exit( 1);
        }

        if ( -1 == ( fd_from_yolo = open( FIFO_FROM_YOLO, O_RDWR)))
        {
            perror( "open() error");
            exit( 1);
        }
    }
    if ( -1 == ( fd_to_yolo = open( FIFO_TO_YOLO, O_RDWR)))
    {
        if ( -1 == mkfifo( FIFO_TO_YOLO, 0666))
        {
            perror( "mkfifo() run error");
            exit( 1);
        }

        if ( -1 == ( fd_to_yolo = open( FIFO_TO_YOLO, O_RDWR)))
        {
            perror( "open() error");
            exit( 1);
        }
    }
    //for STM32
    handle = open( "/dev/ttyTHS2", O_RDWR | O_NOCTTY );
    if( handle < 0 ) 
    {
        //화일 열기 실패
        printf( "Serial Open Fail [/dev/ttyUSB2]\r\n "  );
        exit(0);
    }    
    tcgetattr( handle, &oldtio );  // 현재 설정을 oldtio에 저장

    memset( &newtio, 0, sizeof(newtio) );
    newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD ; 
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;    
    newtio.c_cc[VTIME] = 128;    // time-out 값으로 사용된다. time-out 값은 TIME*0.1초 이다.
    newtio.c_cc[VMIN]  = 0;     // MIN은 read가 리턴되기 위한 최소한의 문자 개수
    
    tcflush( handle, TCIFLUSH );
    tcsetattr( handle, TCSANOW, &newtio );    

    // motor stop
    Buff[0] = 'i';
    write( handle, Buff, 1 );
    printf("i send\n");
  
    // 1 문자씩 받아서 되돌린다. 
    ending = 0;

////////////////////////////////////////////////////////////////
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 1234 );
     
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    listen(socket_desc , 3);
     
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");
     
    //Receive a message from client
    while( (read_size = recv(client_sock, client_message, 2000, 0)) > 0 )
    {
        printf("%s[%d] \n", client_message, read_size);
        //to STM32
        if(!strncmp(client_message, "left", 4))
        {
            Buff[0] = 'a';
            write( handle, Buff, 1 );
            printf("a left\n");
        }
        if(!strncmp(client_message, "right", 5))
        {
            Buff[0] = 'b';
            write( handle, Buff, 1 );
            printf("b right\n");
        }
        if(!strncmp(client_message, "up", 2))
        {
            Buff[0] = 'c';
            write( handle, Buff, 1 );
            printf("c up\n");
        }
        if(!strncmp(client_message, "down", 4))
        {
            Buff[0] = 'd';
            write( handle, Buff, 1 );
            printf("d down\n");
        }
        if(!strncmp(client_message, "stop", 4))
        {
            Buff[0] = 'i';
            write( handle, Buff, 1 );
            printf("i send\n");
        }
        ///////////////////////////////////////////
        //to yolo
//person         0
        if(!strncmp(client_message, "person", 6))
        {
            Buff[0] = 0;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//bicycle        1
        if(!strncmp(client_message, "bicycle", 7))
        {
            Buff[0] = 1;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//car            2
        if(!strncmp(client_message, "car", 3))
        {
            Buff[0] = 2;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//motorbike      3
        if(!strncmp(client_message, "motorbike", 9))
        {
            Buff[0] = 3;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//aeroplane      4
        if(!strncmp(client_message, "aeroplane", 9))
        {
            Buff[0] = 4;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//bus            5
        if(!strncmp(client_message, "bus", 3))
        {
            Buff[0] = 5;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//train          6
        if(!strncmp(client_message, "train", 5))
        {
            Buff[0] = 6;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//truck          7
        if(!strncmp(client_message, "truck", 5))
        {
            Buff[0] = 7;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//boat           8
        if(!strncmp(client_message, "boat", 4))
        {
            Buff[0] = 8;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//traffic light  9
        if(!strncmp(client_message, "traffic light", 13))
        {
            Buff[0] = 9;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//fire hydrant   10
        if(!strncmp(client_message, "fire hydrant", 12))
        {
            Buff[0] = 10;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//stop sign      11
        if(!strncmp(client_message, "stop sign", 9))
        {
            Buff[0] = 11;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//parking meter  12
        if(!strncmp(client_message, "parking meter", 12))
        {
            Buff[0] = 12;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//bench          13
        if(!strncmp(client_message, "bench", 5))
        {
            Buff[0] = 13;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//bird           14 
        if(!strncmp(client_message, "bird", 4))
        {
            Buff[0] = 14;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//cat            15
        if(!strncmp(client_message, "cat", 3))
        {
            Buff[0] = 15;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//dog            16
        if(!strncmp(client_message, "dog", 3))
        {
            Buff[0] = 16;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//horse          17
        if(!strncmp(client_message, "horse", 5))
        {
            Buff[0] = 17;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//sheep          18
        if(!strncmp(client_message, "sheep", 5))
        {
            Buff[0] = 18;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//cow            19
        if(!strncmp(client_message, "cow", 3))
        {
            Buff[0] = 19;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//elephant       20
        if(!strncmp(client_message, "elephant", 8))
        {
            Buff[0] = 20;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//bear           21
        if(!strncmp(client_message, "bear", 4))
        {
            Buff[0] = 21;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//zebra          22
        if(!strncmp(client_message, "zebra", 5))
        {
            Buff[0] = 22;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//giraffe        23
        if(!strncmp(client_message, "giraffe", 7))
        {
            Buff[0] = 23;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//backpack       24
        if(!strncmp(client_message, "backpack", 8))
        {
            Buff[0] = 24;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//umbrella       25
        if(!strncmp(client_message, "umbrella", 8))
        {
            Buff[0] = 25;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//handbag        26
        if(!strncmp(client_message, "handbag", 7))
        {
            Buff[0] = 26;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//tie            27
        if(!strncmp(client_message, "tie", 3))
        {
            Buff[0] = 27;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//suitcase       28
        if(!strncmp(client_message, "suitcase", 8))
        {
            Buff[0] = 28;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//frisbee        29
        if(!strncmp(client_message, "frisbee", 7))
        {
            Buff[0] = 29;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//skis           30
        if(!strncmp(client_message, "skis", 4))
        {
            Buff[0] = 30;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//snowboard      31
        if(!strncmp(client_message, "snowboard", 9))
        {
            Buff[0] = 31;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//sports ball    32
        if(!strncmp(client_message, "sports ball", 11))
        {
            Buff[0] = 32;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//kite           33
        if(!strncmp(client_message, "kite", 4))
        {
            Buff[0] = 33;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//baseball bat   34
        if(!strncmp(client_message, "baseball bat", 12))
        {
            Buff[0] = 34;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//baseball glove 35
        if(!strncmp(client_message, "baseball glove", 14))
        {
            Buff[0] = 35;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//skateboard     36
        if(!strncmp(client_message, "skateboard", 10))
        {
            Buff[0] = 36;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//surfboard      37
        if(!strncmp(client_message, "surfboard", 9))
        {
            Buff[0] = 37;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//tennis racket  38
        if(!strncmp(client_message, "tennis racket", 13))
        {
            Buff[0] = 38;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//bottle         39
        if(!strncmp(client_message, "bottle", 6))
        {
            Buff[0] = 39;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//wine glass     40
        if(!strncmp(client_message, "wine glass", 10))
        {
            Buff[0] = 40;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//cup            41
        if(!strncmp(client_message, "cup", 3))
        {
            Buff[0] = 41;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//fork           42
        if(!strncmp(client_message, "fork", 4))
        {
            Buff[0] = 42;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//knife          43
        if(!strncmp(client_message, "knife", 5))
        {
            Buff[0] = 43;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//spoon          44
        if(!strncmp(client_message, "spoon", 5))
        {
            Buff[0] = 44;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//bowl           45
        if(!strncmp(client_message, "bowl", 4))
        {
            Buff[0] = 45;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//banana         46
        if(!strncmp(client_message, "banana", 6))
        {
            Buff[0] = 46;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//apple          47
        if(!strncmp(client_message, "apple", 5))
        {
            Buff[0] = 47;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//sandwich       48
        if(!strncmp(client_message, "sandwich", 8))
        {
            Buff[0] = 48;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//orange         49
        if(!strncmp(client_message, "orange", 6))
        {
            Buff[0] = 49;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//broccoli       50
        if(!strncmp(client_message, "broccoli", 8))
        {
            Buff[0] = 50;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//carrot         51
        if(!strncmp(client_message, "carrot", 8))
        {
            Buff[0] = 51;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//hot dog        52
        if(!strncmp(client_message, "hot dog", 7))
        {
            Buff[0] = 52;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//pizza          53
        if(!strncmp(client_message, "pizza", 5))
        {
            Buff[0] = 53;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//donut          54
        if(!strncmp(client_message, "donut", 5))
        {
            Buff[0] = 54;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//cake           55
        if(!strncmp(client_message, "cake", 4))
        {
            Buff[0] = 55;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//chair          56
        if(!strncmp(client_message, "chair", 5))
        {
            Buff[0] = 56;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//sofa           57
        if(!strncmp(client_message, "sofa", 4))
        {
            Buff[0] = 57;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//pottedplant    58
        if(!strncmp(client_message, "pottedplant", 11))
        {
            Buff[0] = 58;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//bed            59
        if(!strncmp(client_message, "bed", 3))
        {
            Buff[0] = 59;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//diningtable    60
        if(!strncmp(client_message, "diningtable", 11))
        {
            Buff[0] = 60;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//toilet         61
        if(!strncmp(client_message, "toilet", 6))
        {
            Buff[0] = 61;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//tvmonitor      62
        if(!strncmp(client_message, "tvmonitor", 9))
        {
            Buff[0] = 62;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//laptop         63
        if(!strncmp(client_message, "laptop", 6))
        {
            Buff[0] = 63;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//mouse          64
        if(!strncmp(client_message, "mouse", 5))
        {
            Buff[0] = 64;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//remote         65
        if(!strncmp(client_message, "remote", 6))
        {
            Buff[0] = 65;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//keyboard       66
        if(!strncmp(client_message, "keyboard", 8))
        {
            Buff[0] = 66;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//cell phone     67
        if(!strncmp(client_message, "cell phone", 10))
        {
            Buff[0] = 67;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//microwave      68
        if(!strncmp(client_message, "microwave", 9))
        {
            Buff[0] = 68;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//oven           69
        if(!strncmp(client_message, "oven", 4))
        {
            Buff[0] = 69;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//toaster        70
        if(!strncmp(client_message, "toaster", 7))
        {
            Buff[0] = 70;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//sink           71
        if(!strncmp(client_message, "sink", 4))
        {
            Buff[0] = 71;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//refrigerator   72
        if(!strncmp(client_message, "refrigerator", 12))
        {
            Buff[0] = 72;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//book           73
        if(!strncmp(client_message, "book", 4))
        {
            Buff[0] = 73;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//clock          74
        if(!strncmp(client_message, "clock", 5))
        {
            Buff[0] = 74;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//vase           75
        if(!strncmp(client_message, "vase", 4))
        {
            Buff[0] = 75;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//scissors       76
        if(!strncmp(client_message, "scissors", 8))
        {
            Buff[0] = 76;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//teddy bear     77
        if(!strncmp(client_message, "teddy bear", 10))
        {
            Buff[0] = 77;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//hair drier     78
        if(!strncmp(client_message, "hair drier", 10))
        {
            Buff[0] = 78;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }
//toothbrush     79         
        if(!strncmp(client_message, "toothbrush", 10))
        {
            Buff[0] = 79;
            write( fd_to_yolo, Buff, 1);
            printf("%d send\n", Buff[0]);
        }

#if 0
        if(!strncmp(client_message, "apple", 5))
        {
            Buff[0] = 'A';
            write( fd_to_yolo, Buff, 1 );
            printf("%c send\n", Buff[0]);
        }
/*     
        if(!strncmp(client_message, "banana", 6))
        {
            Buff[0] = 'B';
            write( fd_to_yolo, Buff, 1 );
            printf("%c send\n", Buff[0]);
        }
*/
        if(!strncmp(client_message, "person", 6))
        {
            Buff[0] = 'B';
            write( fd_to_yolo, Buff, 1 );
            printf("%c send\n", Buff[0]);
        }
        if(!strncmp(client_message, "bicycle", 7))
        {
            Buff[0] = 'C';
            write( fd_to_yolo, Buff, 1 );
            printf("%c send\n", Buff[0]);
        }
        if(!strncmp(client_message, "dog", 3))
        {
            Buff[0] = 'D';
            write( fd_to_yolo, Buff, 1 );
            printf("%c send\n", Buff[0]);
        }
        if(!strncmp(client_message, "truck", 5))
        {
            Buff[0] = 'E';
            write( fd_to_yolo, Buff, 1 );
            printf("%c send\n", Buff[0]);
        }
#endif
        memset(client_message, 0, sizeof(client_message));
    }

    Buff[0] = 'i';
    write( handle, Buff, 1 );
    printf("i send\n");
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    close(socket_desc);
    close( fd_from_yolo);
    close( fd_to_yolo);
    // 쓰레드 종료를 기다린다. 
    //pthread_join(p_thread[0], (void **)&status);
     
    return 0;
}

