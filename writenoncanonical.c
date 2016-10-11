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

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int flag=1, conta=1;

void atende()                   // atende alarme
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
}

char writeSet(int fd,int manhoso)
{
	int res;	
	//char set[5] = {0x7E,0x03,0x03,0x00,0x7E};
	
	
	char set[5] = {0x7E,0x03,0x03,0x02,0x7E}; // TA MALE!
	char set2[5] = {0x7E,0x03,0x03,0x00,0x7E};

	
	if( manhoso == 0){
	res = write(fd,set,5);}
	else{
	res = write(fd,set2,5);}

	
	char buf[1];

	printf("%d bytes written\n", res);	
}

char readUa(int fd)
{
	char buf[1];
	char ua[5] = {0x7E,0x03,0x07,0x01,0x7E};
	int res;
	int counter = 0;

	
	while(conta < 4 && STOP == FALSE)
	{

		if(flag)
		{  		
			alarm(3); // 3 second alarm
			flag=0;
		}
		
		while (STOP==FALSE) 
		{	
			printf("start raeding \n");
			res = read(fd,buf,1);
			printf("%d",res);
			if(res == 0)
			{
				//printf("Nsei");
				//flag =1;
				break;
			}
			//conta = 0;
		switch(counter)
		{
		case 0:
			if(buf[0]==ua[0])
			printf("SUCCESS\n");
			else printf("ERROR\n");
			break;
		case 1:
			if(buf[0]==ua[1])
			printf("SUCCESS\n");
			else printf("ERROR\n");
			break;
		case 2:
			if(buf[0]==ua[2])
			printf("SUCCESS\n");
			else printf("ERROR\n");
			break;
		case 3:
			if(buf[0]==ua[3])
			printf("SUCCESS\n");
			else printf("ERROR\n");
			break;
		case 4:
			if(buf[0]==ua[4])
			printf("SUCCESS\n");
			else printf("ERROR\n");
			break;
			
		};  		
	
  		counter++;
  		buf[1]='\0';	
  		if (counter==5)
		{ 
			
	 		STOP=TRUE;

  		}
	}
		
		
	}

	if(conta >=4)
		{			
			printf("Reception Error \n");	
			writeSet(fd,1);
		}

	
}


char llopen(int fd)
{
	writeSet(fd,0);
	readUa(fd);
	
}
int main(int argc, char** argv)
{

	(void) signal(SIGALRM, atende); // ENABLES ALARM SIGNALS
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



