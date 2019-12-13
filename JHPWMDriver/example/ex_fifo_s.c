// ex_fifo_s.c
// gcc -o ex_fifo_s ex_fifo_s.c
#include <sys/types.h>               // 헤더 파일
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


#define  FIFO_FILE   "/tmp/fifo"

int main_menu(void);
static unsigned short flag;

int main()
{
    int i, key;
    char buffer1[]="write";
    char buffer2[256];


    int fd;
    char *str   = "system programming";

    printf("0000000000000000\n");
    if ((fd = open(FIFO_FILE, O_RDWR)) < 0)           //  fifo 파일 열기
    {
        printf( "1111111111111\n");
        printf( "1111111111111\n");
        if (mkfifo(FIFO_FILE, 0666) < 0)                    // fifo 파일 생성
        {
            printf("mkfifo() run error\n");
        }
        printf( "2111111111111\n");
        if ((fd = open(FIFO_FILE, O_RDWR)) < 0)           //  fifo 파일 열기
        {


            printf("3111111111111\n");
            perror("open() error");
            exit(1);
        }
    }

        printf("3111111111111\n");

    while((key=main_menu()) != 0)
    {
        switch(key)
        {
            case 'r':
                printf("No.1\n");
                buffer1[0] = 'r';
                write(fd, buffer1, 1);                             //  write to fifo
                break;
                            
            case 'w':
                printf("No.1\n");
                buffer1[0] = 'w';
                write(fd, buffer1, 1);                             //  write to fifo
                break;               

            case 'e':
                printf("No.2\n");
                buffer1[0] = 'e';
                write(fd, buffer1, 1);                             //  write to fifo
                break;

            case 'a':
                printf("No.3\n");
                buffer1[0] = 'a';
                write(fd, buffer1, 1);                             //  write to fifo
                break;

           case 's':
                printf("No.4\n");
                buffer1[0] = 's';
                write(fd, buffer1, 1);                             //  write to fifo
                break;

            case 'd':
                printf("No.5\n");
                buffer1[0] = 'd';
                write(fd, buffer1, 1);                             //  write to fifo
                break;

            case 'x':
                printf("No.6\n");
                buffer1[0] = 'x';
                write(fd, buffer1, 1);                             //  write to fifo
                break;

            case 'c':
                printf("No.7\n");
                buffer1[0] = 'c';
                write(fd, buffer1, 1);                             //  write to fifo
                break;

            case 'q':
                printf("No.8\n");
                buffer1[0] = 'q';
                write(fd, buffer1, 1);                             //  write to fifo
                printf("exit\n");
                close(fd);
                exit(0);
                break;
        }
    }
    close(fd);
    return 0;
}

int main_menu(void)
{
    int key;
    
    printf("\n\n");
    printf("-------------------------------------------------\n");
    printf("                    MAIN MENU\n");
    printf("-------------------------------------------------\n");
    printf(" r. high go             \n");
    printf(" w. mld go             \n");
    printf(" e. low go             \n");
    printf(" a. left             \n");
    printf(" s. back             \n");
    printf(" d. right             \n");
    printf(" x. center                \n");
    printf(" c. stop               \n");

    printf("-------------------------------------------------\n");
    printf(" q.QUIT\n");
    printf("-------------------------------------------------\n");
    printf("\n\n\n");
    
    printf("SELECT THE COMMAND NUMBER : ");
    key=getchar();
    
    if (key == '\n') 
        ;
    else
        getchar();
    return key;
}

