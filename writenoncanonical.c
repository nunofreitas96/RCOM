/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int flag=0, conta=1;// contalarme=1;

void atende()                   // atende alarme
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
	
}

void writeSet(int fd)
{
	srand(time(NULL));
	int random = rand()%3+1;
	int res;	
	char set[5] = {0x7E,0x03,0x03,0x00,0x7E};
	char set3[5] = {0x7E,0x03,0x03,0x01,0x7E};

	switch(random) 
	{
		case 1:
		printf("PRINTING RIGHT SET\n");
		res = write(fd,set,5);
		break;
		case 2:
		printf("PRINTING RIGHT SET\n");
		res = write(fd,set,5);
		break;
		case 3:
		printf("PRINTING WRONG SET\n");
		res = write(fd,set3,5);
		break;
		default:
		printf("?????\n");
		break;
	}
	
	

	printf("%d bytes written\n", res);	

}

int readUa(int fd)
{
	char buf[1];
	char ua[5] = {0x7E,0x03,0x03,0x01,0x7E};
	int res;
	int counter = 0;
	int errorflag =0;

	while (STOP==FALSE && counter < 5) 
	{
		res = read(fd,buf,1);
				
		switch(counter)
		{
			case 0:
				if(buf[0]==ua[0])
				printf("SUCCESS\n");
				else
				{ 
					printf("ERROR\n");
					errorflag=-1;
				}
				break;
			case 1:
				if(buf[0]==ua[1])
				printf("SUCCESS\n");
				else
				{ 
					printf("ERROR\n");
					errorflag=-1;
				}
				break;
			case 2:
				if(buf[0]==ua[2])
				printf("SUCCESS\n");
				else
				{ 
					printf("ERROR\n");
					errorflag=-1;
				}
				break;
			case 3:
				if(buf[0]==ua[3])
				printf("SUCCESS\n");
				else
				{ 
					printf("ERROR\n");
					errorflag=-1;
				}
				break;
			case 4:
				if(buf[0]==ua[4])
				printf("SUCCESS\n");
				else
				{ 
					printf("ERROR\n");
					errorflag=-1;
				}
				break;
		};  		
  		counter++;

  		if (counter==5 && errorflag ==0)
		{ 
	 		STOP=TRUE;
  		}
	}
	return res;
}


int llopen(int fd)
{
	
	int res = 0;

	
		while(conta < 4)
		{
			writeSet(fd);
			alarm(3);
			res = readUa(fd);
			if (res == 0 || res==-1) //fails to read ua
			{
				flag = 0;
			}
			else{ 
			alarm(0);
			return 0;				
			}
			while(!flag && STOP == FALSE)
			{}
		}
	return -1;
}
int main(int argc, char** argv)
{

	(void) signal(SIGALRM, atende); // ENABLES ALARM SIGNALS
    int fd;
    struct termios oldtio,newtio;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 3;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */


	
    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    //printf("Message to send: ");
	
	llopen(fd);

/*
	gets(buf);
    
   	int tam = strlen(buf);
    
    res = write(fd,buf,tam+1);
    printf("%d bytes written\n", res);
	
	//sleep(1);

	i = 0;
    while (STOP==FALSE) {
      res = read(fd,buf_res+i,1); 
      i++;
      if(buf_res[i-1] == '\0') 
         STOP = TRUE;    	
    }

	printf("Message received: %s\n", buf_res);*/

  /* 
    O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar 
    o indicado no guião 
  */

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
	
}



