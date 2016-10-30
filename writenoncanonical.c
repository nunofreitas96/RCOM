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

char C1 = 0x00;

FILE *fp;

volatile int STOP=FALSE;

int flag=0, conta=1;

void switchC1()
{
	if (C1 == 0x00)
	C1 = 0x40;
	else
	C1 = 0x00;
	return ;
}
void atende()
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;

}

int writeBytes(int fd)
{
	char ua[5] = {0x7E,0x03,0x03,0x01,0x7E};
  int size=strlen(ua);
	int res;

	res = write(fd,ua,5);
	printf("%d bytes written\n", res);
	return res;
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



int sendInfoFile(int fd, unsigned char *buf, int size) //Handles the process of sending portions of the file to receiver
{
	int newSize = (size+6),i,j,res;
	unsigned char BCC2,BCC1;

	for (i = 0; i < size; i++)
	{
		if (buf[i] == 0x7E || buf[i] == 0x7D)
		{
			newSize++;
		}
	}

	//COMPUTING BCC2
	BCC2 = buf[0] ^ buf[1];
	for (i = 2; i < size;i++)
	{
		BCC2 = BCC2^buf[i];
	}

	unsigned char *dataPacket = (unsigned char*)malloc(newSize);

	dataPacket[0] = FLAG;
	dataPacket[1] = 0x03;
	dataPacket[2] = C1;
	BCC1 = dataPacket[1]^C1;
	dataPacket[3] = BCC1;

	int k;
	i = 4;
	j = i+1;
	for (k = 0; k < size;k++)
	{
		if (buf[k] == 0x7E)
		{
				dataPacket[i] = 0x7D;
				dataPacket[j] = 0x5E;
		}

		if(buf[k] == 0x7D)
		{
				dataPacket[i] = 0x7D;
				dataPacket[j] = 0x5D;
		}
		dataPacket[i] = (unsigned char)buf[k];
		i++;
		j++;
	}

	dataPacket[newSize-2] = BCC2;
	dataPacket[newSize-1] = FLAG;

	i = 0;
	for(; i < newSize;i++)
	{
		printf("dataPacket[%d] = 0x%02X\n",i,(unsigned char)dataPacket[i]);
	}

	res = write(fd,dataPacket,newSize);
	switchC1();
	return res;
}

int getDataPacket(int fd) //Handles the process of dividing file into 512 bytes portions
{
	int i = 0, sizeDataPacket = 0;
	fp = fopen("pinguim.gif","rb");
	int bytesRead = 0,res,read = 0;

	unsigned char *dataPacket = (unsigned char *)malloc(512);

	while ((bytesRead = fread(dataPacket, sizeof(unsigned char), 512, fp)) > 0)
	{
		read += bytesRead;
		sendInfoFile(fd,dataPacket,bytesRead);

		//make dataPacket ready for next iteration
		dataPacket = memset(dataPacket, 0, 512);
	}
	fclose(fp);
	return read;
}

/*
 *This fucntion will read or send Disconnect
*/
int sendReadDISC(int fd,int toRead)
{
	int counter = 0;
	int errorflag =0;
	char disc[5];
	char buf[5];

	char A=0x03;
	char C = 0x0B;
	char BCC1 = A^C;

	disc[0] = FLAG;
	disc[1] = A;
	disc[2] = C;
	disc[3] = BCC1;
	disc[4] = FLAG;

	int res = 0;

	if (toRead == TRUE)
	{
		while (STOP==FALSE && counter < 5)
		{
			res = read(fd,buf,1);

			switch(counter)
			{
				case 0:
				if(buf[0]!=disc[0])
					errorflag=-1;
				break;
				case 1:
				if(buf[0]!=disc[1])
					errorflag=-1;
				break;
				case 2:
				if(buf[0]!=disc[2])
					errorflag=-1;
				break;
				case 3:
				if(buf[0]!=disc[3])
					errorflag=-1;
				break;
				case 4:
				if(buf[0]!=disc[4])
					errorflag=-1;
				break;
			};
			counter++;

			if (counter== 5 && errorflag == 0)
			{
				STOP=TRUE;
				return 0;
			}
			else if (counter == 5 && errorflag != 0)
				return -1;
		}
	}
	else
	{
		res = write(fd,disc,5);
		return 0;
	}
}

int readREJ(int fd)
{
	int counter = 0;
	int errorflag =0;
	char rej[5];
	char buf[5];

	char A=0x03;
	char C = 0x01;
	char BCC1 = A^C;

	rej[0] = FLAG;
	rej[1] = A;
	rej[2] = C;
	rej[3] = BCC1;
	rej[4] = FLAG;

	int res = 0;

	while (STOP==FALSE && counter < 5)
	{
		res = read(fd,buf,1);

		switch(counter)
		{
			case 0:
			if(buf[0]!=rej[0])
				errorflag=-1;
			break;
			case 1:
			if(buf[0]!=rej[1])
				errorflag=-1;
			break;
			case 2:
			if(buf[0]!=rej[2])
				errorflag=-1;
			break;
			case 3:
			if(buf[0]!=rej[3])
				errorflag=-1;
			break;
			case 4:
			if(buf[0]!=rej[4])
				errorflag=-1;
			break;
		};
		counter++;

		if (counter==5 && errorflag ==0)
		{
			STOP=TRUE;
			return 0;
		}
	}
	return -1;
}

int ReadRR(int fd)
{
	int counter = 0;
	int errorflag =0;
	char rr[5];
	char buf[5];

	char A=0x03;
	char C = 0x05;
	char BCC1 = A^C;

	rr[0] = FLAG;
	rr[1] = A;
	rr[2] = C;
	rr[3] = BCC1;
	rr[4] = FLAG;

	int res = 0;

	while (STOP==FALSE && counter < 5)
	{
		res = read(fd,buf,1);

		switch(counter)
		{
			case 0:
			if(buf[0]!=rr[0])
				errorflag=-1;
			break;
			case 1:
			if(buf[0]!=rr[1])
				errorflag=-1;
			break;
			case 2:
			if(buf[0]!=rr[2])
				errorflag=-1;
			break;
			case 3:
			if(buf[0]!=rr[3])
				errorflag=-1;
			break;
			case 4:
			if(buf[0]!=rr[4])
				errorflag=-1;
			break;
		};
		counter++;

		if (counter==5 && errorflag ==0)
		{
			STOP=TRUE;
			return 0;
		}
	}
	return -1;
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
			return 0;
		}
	}
	return -1;
}

char *buildStartPacket(int fd)
{
	int fsize, aux1, recoveredFileSize=0, i=0, j=0;
	char *fileName = "pinguim.gif";
	char sz[4]={'0','0','0','0'};
	fseek(fp,0,SEEK_END);
	fsize = ftell(fp);
	fseek(fp,0,SEEK_SET);
	aux1 = fsize;

	//printf("%X\n\n",(char *)fsize);

	char A = 0x03;
	char BCC1 = A^C1;
	char BCC2;

	fclose(fp);

	//int startBufSize = 9+strlen(fileName);
	int startBufSize = 9+strlen(fileName)+1;

	char *startBuf = (char *)malloc(startBufSize);

	startBuf[0] = 0x02;
	startBuf[1] = 0x00;
	startBuf[2] = 0x04;
	int* intloc= (int*)(&startBuf[3]);
	*intloc=fsize;
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

	//COMPUTE FINAL SIZE OF DATA ARRAY
	int sizeFinal = startBufSize;
	for (i = 0; i < startBufSize;i++)
	{
		if (startBuf[i] == 0x7E || startBuf[i] == 0x7D)
			sizeFinal++;
	}
	printf("%lu \n",sizeFinal);

	for (i = 0; i < startBufSize;i++)
		printf("startBuf[%d] = %02X\n",i,startBuf[i]);

	char *blet = (unsigned char *)malloc(sizeFinal);

	j = 1;
	int k=0;
	for (i = 0; i < startBufSize;i++)
	{
		if (startBuf[i] == 0x7E)
		{
			blet[k] = 0x7D;
			blet[j] = 0x5E;
			k++;
			j++;
		}

		if (startBuf[i] == 0x7D)
		{
			blet[k] = 0x7D;
			blet[j] = 0x5D;
			k++;
			j++;
		}

		if(startBuf[i] != 0x7D && startBuf[i] != 0x7E)
			blet[k] = startBuf[i];

		j++;
		k++;
	}

	printf("after stuff %02x\n",blet[20]);
	for (i = 0; i < sizeFinal;i++)
		printf("blet[%d] = %02X\n",i,blet[i]);

	//COMPUTING BCC2
	BCC2 = startBuf[0] ^ startBuf[1];
	for (i = 2; i < startBufSize;i++)
	{
		BCC2 = BCC2^startBuf[i];
	}

	//STUFFING OF DATA PACKAGE
	char *dataPackage = (char *)malloc(sizeFinal);
	dataPackage[0] = FLAG;
	dataPackage[1] = A;
	dataPackage[2] = C1;
	BCC1 = A^C1;
	dataPackage[3] = BCC1;

	//BUILDING DATA PACKAGE AND INSERTING IN "I" PACKET
/*	j = 5;
	k = 0;
	for (i = 4; i < sizeFinal;i++)
	{
		if (startBuf[k] == 0x7E)
		{
			dataPackage[i] = 0x7D;
			dataPackage[j] = 0x5E;
		}
		else if (startBuf[k] == 0x7D)
		{
			dataPackage[i] = 0x7D;
			dataPackage[j] = 0x5D;
		}
		else
			dataPackage[i] = startBuf[k];
		j++;
		k++;
	}*/

	//ISTO ESTA ASSIM FOR DA SAKE OF TESTING
	dataPackage[sizeFinal-2] = BCC2;
	dataPackage[sizeFinal-1] = FLAG;

	int res = 0;
	res = write(fd, dataPackage, sizeFinal);

	/*i = 0;
	for(;i < sizeFinal; i++)
	{
		printf("dataPackage[%d] = 0x%02X\n",i,(unsigned char)dataPackage[i]);
	}*/

	printf("%d bytes written\n",res);
	return 0;
}

void cycle(int fd)
{
	int fsize;
	fp = fopen("pinguim.gif","rb");
	fseek(fp,0,SEEK_END);
	fsize = ftell(fp);
	fseek(fp,0,SEEK_SET);
	fclose(fp);

	//llopen(); //SET AND UA PLACE

	/*while( < )
	{

	}*/
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
	leitura do(s) pr�ximo(s) caracter(es)
	*/



tcflush(fd, TCIOFLUSH);

if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
	perror("tcsetattr");
	exit(-1);
}

	//printf("Message to send: ");
	buildStartPacket(fd);
	//getDataPacket(fd);
	//cycle(fd);
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
O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar
o indicado no gui�o
*/




if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
	perror("tcsetattr");
	exit(-1);
}

close(fd);
return 0;

}
