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
#include <math.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define A 0x03
#define C1 0x00
#define C2 0x40
#define BCC1

FILE *fp;

volatile int STOP=FALSE;

int flag=0, conta=1;

void atende()
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
			if(buf[0]!=ua[0])
				errorflag=-1;
			break;
			case 1:
			if(buf[0]!=ua[1])
				errorflag=-1;
			break;
			case 2:
			if(buf[0]!=ua[2])
				errorflag=-1;
			break;
			case 3:
			if(buf[0]!=ua[3])
				errorflag=-1;
			break;
			case 4:
			if(buf[0]!=ua[4])
				errorflag=-1;
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

char *buildStartPacket()
{
	int fsize, aux1, recoveredFileSize=0, i=0, j=0;
	char *fileName = "pinguim.gif";
	char sz[4]={'0','0','0','0'};
	fseek(fp,0,SEEK_END);
	fsize = ftell(fp);
	fseek(fp,0,SEEK_SET);
	aux1 = fsize;

	while(aux1>0xFF)
	{
		aux1=fsize/(pow(255.0,i+1));
		sz[i]=fsize/(pow(255.0,i)-aux1*255);
		i++;
	}

	if(i<3)
	{
		sz[i]=aux1;
		for(j=i+1;j<3;j++) 
			sz[j]=0;
	}

	fclose(fp);
	
	int startBufSize = 9+strlen(fileName);

	char *startBuf = (char *)malloc(startBufSize);
	startBuf[0] = 0x02;
	startBuf[1] = 0x00;
	startBuf[2] = 0x04;
	startBuf[3] = sz[0];
	startBuf[4] = sz[1];
	startBuf[5] = sz[2];
	startBuf[6] = sz[3];
	startBuf[7] = 0x01;
	startBuf[8] = strlen(fileName);
	startBuf[9] = 'p';
	startBuf[10] = 'i';
	startBuf[11] = 'n';
	startBuf[12] = 'g';
	startBuf[13] = 'u';
	startBuf[14] = 'i';
	startBuf[15] = 'm';
	startBuf[16] = '.';
	startBuf[17] = 'g';
	startBuf[18] = 'i';
	startBuf[19] = 'f';

	//fazer stuffing	x
	//colocar numa trama I	x
	//enviar		x
	
	char *stuffingStart = (char *)malloc(startBufSize+5);
	i = 0;
	for (; i < startBufSize;i++)
	{
		stuffingStart[i] = startBuf[i];
	}
	stuffingStart[startBufSize+1] = A^C1; //FALTA VERIFICAR ISTO
	printf("stuffingStart[] = 0x%X\n",stuffingStart[startBufSize+1]);

	i = 0;
	/*for (; i < startBufSize+1;i++)
	{
		stuffingStart[i] = startBuf[i];
		printf("stuffingStart[%d] = 0x%X\n",i,stuffingStart[i]);
	}*/


	return 0;
}

int llwrite(int fd)
{
	
	return 0;
}

int llread(int fd)
{
	return 0;	
}


int llopen(int fd)
{
	while(conta < 4)
	{
		writeSet(fd);
		alarm(3);	
		
		while(!flag && STOP == FALSE)
			{	readUa(fd);	}

		if(STOP==TRUE)
		{
			alarm(0);
			return 0;				
		}		
		else 
			flag=0;
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

	fp = fopen("pinguim.gif","r");
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
buildStartPacket();
	//llwrite(fd);	
	//llopen(fd);


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



