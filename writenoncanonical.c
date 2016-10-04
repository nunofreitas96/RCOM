/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

char llopen(){

	int res;
	int res2;
	char set[5] = 0x7E0303007E;
	char ua[5] = 0x7E0307017E;
	int tam = strlen(set);
	res = write(fd,set,tam);
	char buf[5];

	printf("%d bytes written\n", res);

	int i = 0;
	int i2 = 0;
	while (STOP==FALSE) 
	{
	  res2 = read(fd,buf,1);
	  if (buf[i] == 0x7E)
		{	
			if (i2 == 1)
				break;
			i2++;
			i++;
			continue;			
		}
 
	  if(ua[i] != buf[i])
		{
			STOP = TRUE;
		}
	  i++;

	}
	printf("Message received: %s\n", buf);
}

int main(int argc, char** argv)
{

    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255], buf_res[255];
    int i, sum = 0, speed = 0, len = 0;
    
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

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */



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
	
	llopen();

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



