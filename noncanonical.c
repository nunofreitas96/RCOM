/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define ERR 0x36
#define ERR2 0x35


int status = TRUE;
volatile int STOP=FALSE;
volatile int readFile=FALSE;
volatile int readStart = FALSE;
//TODO retirar por problemas de memória

typedef struct{
	char arr[5];
} ResponseArray;

typedef struct{
	char* arr;
	int size;
} DataPack;

typedef struct{
	char* arr;
	int namelength;
	int fileSize;
} FileData;


//char* file_name;
FileData file;
FILE *fp;
//Funções GUIA1
void writeBytes(int fd, char* message){

	printf("SendBytes Initialized\n");
    int size=strlen(message);
	int sent = 0;

    while( (sent = write(fd,message,size+1)) < size ){
        size -= sent;
    }

}


char * readBytes(int fd){
   	char* collectedString=malloc (sizeof (char) * 255);
	char buf[2];
	int counter=0,res=0;
    while (STOP==FALSE) {       /* loop for input */

	  res = read(fd,buf,1);   /* returns after 1 chars have been input */

	  if(res==-1)
		exit(-1);
	  buf[1]='\0';
      collectedString[counter]=buf[0];

      if (buf[0]=='\0'){
	  collectedString[counter]=buf[0];
	  STOP=TRUE;
      }
      counter++;
    }
    printf("end result:%s\n",collectedString);
	return collectedString;

}

//FUNÇÕES GUIA 2
char readSupervision(int fd, int counter){

	char set[5]={0x7E,0x03,0x03,0x00,0x7E};
	char buf[1];
	int res=0;
    	res = read(fd,buf,1);   /* returns after 1 chars have been input */

	if(res==-1){
	printf("read error\n");
	return ERR;
	}

	switch(counter){
	case 0:
		if(buf[0]==set[0]){
			return 0x76;
		}
		return ERR;
	case 1:
		if(buf[0]==set[1]){
			return 0x03;
		}
		return ERR;
	case 2:
		if(buf[0]==set[2]){
			return 0x03;
		}
		return ERR;
	case 3:
		if(buf[0]==set[3]){
			return buf[0];
		}
		return ERR2;
	case 4:
		if(buf[0]==set[4]){
			return 0X7E;
		}
		return ERR;
  default:
		return ERR;
	}
}


void llopen(int fd, int type){
 char ua[5]={0x7E,0x03,0x03,0x01,0x7E};
 char readchar[2];
 int counter = 0;
 if(type==0){
	 while (STOP==FALSE) {       /* loop for input */

	  readchar[0]=readSupervision(fd,counter);
	  printf("%c \n",readchar[0]);
	  readchar[1]='\0';


	  counter++;

	  if(readchar[0]==ERR){
	  counter=0;
	  }

	  if(readchar[0]==ERR2){
	  counter=-1;
	  }

	  if (counter==5){
		 STOP=TRUE;
	  }

	 }
 }
	printf("Sending UA...\n");
    writeBytes(fd,ua);
}

DataPack destuffPack(DataPack todestuff)
{
	printf("BEFORE DESTUFF size:%d  \n",todestuff.size);
	//TODO make this more accessible to other functions
	char* dbuf=malloc(todestuff.size);
	DataPack startPack;
	// counter for finding all bytes to destuff, starts on 4 because from 0 to 3 is the header
	//j is a counter to put bytes on dbuf;
	int i;
	for(i=0;i<4;i++){
		dbuf[i]=todestuff.arr[i];
	}

	int j =4;

	//Finding content that needs destuffing
	while(i<todestuff.size)
	{
		printf(" destuff[i]= %x , i:%d j:%d\n",todestuff.arr[i],i,j);
		//no flags
		if(todestuff.arr[i] != 0x7E && todestuff.arr[i] != 0x7D){

		 dbuf[j] = todestuff.arr[i];
		 i++;
		 j++;
		 continue;
		}


		if(todestuff.arr[i] == 0x7E){
			if(i!=(todestuff.size-1))
					//return err;
			 dbuf[j] = todestuff.arr[i];
			break;
		}

		if(todestuff.arr[i] == 0x7D){
			if(todestuff.arr[i+1] == 0x5E){
				dbuf[j] = 0x7E;
				i = i+2;
				j++;
				if(i >= todestuff.size)
				{
					 printf("index out of bound");
					//return ERR;
				}
				continue;
		 	}

			if(todestuff.arr[i+1] == 0x5d){
				dbuf[j] = 0x7D;
				i = i+2;
				j++;
					if(i >= todestuff.size)
					{
					 printf("index out of bound");
					 //return ERR;
				  }
				continue;
			}

			//return err;
		}

	}

	startPack.size=j+1;
  startPack.arr=dbuf;
	printf("AFTER DESTUFF Size:%d \n",startPack.size);



	return startPack;
}

void printArray(char* arr,size_t length){

	int index;
	for( index = 0; index < length; index++){
			printf( "0x%X\n", (unsigned char)arr[index] );
	}
}

ResponseArray readInfPackHeader(int fd, char* buf){

	//Verifying that the header of the package is correct
	ResponseArray response;
	char c1alt;
	char REJ[5]={0x7E,0x03,0x01,0x03^0x01,0x7E};
	char restartERR2[5]={ERR2,ERR2,ERR2,ERR2,ERR2};


	//Verifying starting flag
	if(buf[0] != 0x7E){
		printf("first byte isn't flag error \n");
		memcpy(response.arr,REJ,5);
		return response;
	}

	//Verifying A
	if(buf[1] != 0x03){
		printf("read error in (A) \n");
		memcpy(response.arr,REJ,5);
		return response;
	}

	//Verifying C1
	if(buf[2] != 0x00 && buf[2] != 0x40){
		if(buf[2]== 0x03){
			if(buf[3]==0x00){
				llopen(fd,1);
				memcpy(response.arr,restartERR2,5);
				return response;
			}
		}
		printf("read error in (C)");
		memcpy(response.arr,REJ,5);
		return response;
	}

	//Verifying BCC1
	if((buf[1]^buf[2]) != buf[3]){
		printf("A^C is not equal to BCC1 error");
	  memcpy(response.arr,REJ,5);
		return response;

	}

	//Alternating C1
	if(buf[2] == 0x00){
		c1alt = 0x40;
	}
	else if(buf[2] == 0x40){
		c1alt = 0x00;
	}
	//criating header of start package to send
	char RR[5] = {0x7E,0x03,c1alt,0x03^c1alt,0x7E};
	memcpy(response.arr,RR,5);
	printf("Received header with no errors\n");

	return response;
}


ResponseArray readStartPacketInfo(char * startPacket, ResponseArray res)
{
	char tempI[5];
	char temp[50];
	int offset = 4;
	char REJ[5]={0x7E,0x03,0x01,0x03^0x01,0x7E};

	tempI[0]=startPacket[offset+6];
	tempI[1]=startPacket[offset+5];
	tempI[2]=startPacket[offset+4];
	tempI[3]=startPacket[offset+3];

	int currentI=offset+6;
	sprintf(temp,"%02x%02x%02x%02x",(unsigned char)tempI[0],(unsigned char)tempI[1],(unsigned char)tempI[2],(unsigned char)tempI[3]);
	printf("%s\n",temp);
	//output is 00002ad8
	file.fileSize=strtol(temp,NULL,16);
	printf("FILESIZE IS %d \n",file.fileSize);
	if(file.fileSize<0 || file.fileSize>4*pow(10,9))
	{
		memcpy(res.arr,REJ,5);
		return res;
	}

	int fileNameSize = startPacket[currentI+2];
	printf("FILE NAME SIZE IS IS %d \n",fileNameSize);
	file.arr=malloc(fileNameSize+1);

	if(fileNameSize<=0)
	{
		memcpy(res.arr,REJ,5);
		return res;
	}

	int i =0;

	while( i < fileNameSize)
	{
		file.arr[i] = startPacket[currentI+3+i];
		i++;
	}



	//verify BCC2
	printf("FILE NAME IS IS %s \n",file.arr);

	return res;
}



void validateStartPack(int fd){

 	DataPack sp;
  sp.size=50;
	sp.arr=malloc(sp.size);


	int res=-1;
  int counter=0;
  int first7E = FALSE;

	while(counter<40)
	{
	 	res = read(fd,&sp.arr[counter],1);

		if(res==-1)
		{
			printf("ERROR READING: QUITTING\n");
		}
	// 	printf("CURRENT CHARACTER:0x%02x\n",(unsigned char)readchar[counter]);


	 	if(first7E==TRUE){
	 		if(sp.arr[counter]==0x7E){
	 			printf("FOUND 2ND 7E %d\n",counter);
					sp.size=counter+1;
	 			  sp.arr = realloc(sp.arr,sp.size);
					printf("SIZEOF sp.arr AFTER REALLOC %d\n",sp.size);
	 			break;
	 		}
	 	}

	 	if(sp.arr[counter]==0x7E){
	 			first7E=TRUE;
	 	}
			counter++;
  }


	printArray(sp.arr,sp.size);


	ResponseArray response =readInfPackHeader(fd,sp.arr);
	//read first 4 bytes to sp.arr, send sp.arr to readInfpacketHeader


	if(response.arr[0]==ERR2)
	{
		printf("Detected SET, Resent UA, going to try and read new Start Pack\n");
		readStart=FALSE;
		return;
	}


  if(response.arr[2]==0x01){
		readStart=FALSE;
		return;
	}


	switch(sp.arr[2])
	{
		case 0x00:
		printf("Validated Starter Packet Header, gotta break it down now\n");
		//DataPack startPack = destuffPack(fd, sp.arr, strlen(sp.arr)-4);

		response = readStartPacketInfo(sp.arr,response);
		printf("reading the response that I received from readStartPAcketInfo\n");
		printArray(response.arr,5);
		printf("END OF RESPONSE READ2323:\n");
		writeBytes(fd,response.arr);
		if(response.arr[2]==0x01){
			readStart=FALSE;
			return;
		}
		readStart=TRUE;
		break;

    default:
		printf("Rejecting invalid Starter Packet, try again \n");
		printArray(response.arr,5);
		writeBytes(fd,response.arr);
		readStart=FALSE;
		break;
	 }
  }

  
void readFileInfo(int fd, int size){
	char* buf;
	int res =0;
	
	read(fd, buf, size);
	
	ResponseArray responsePack;
	DataPack dataPack;
	dataPack.arr = buf;
	responsePack = readInfPackHeader(fd, buf);
	dataPack = destuff(dataPack);
	
	
	
	fwrite(dataPack.arr,1,dataPack.size,fp);
	
	
}
  
void llread(int fd)
{
	 char readchar[4];

	//TRYING TO READ JUST THE START PACKET
	while(readStart == FALSE)
	{

		validateStartPack(fd);

		if(readStart==TRUE)
		{

			//reading actual file
		 	 while (readStart==TRUE)
			{
				//read first 4 bytes to readchar, send readchar to readInfpacketHeader
				ResponseArray response =readInfPackHeader(fd,readchar);
				if(response.arr[0]==ERR2)
				{
				  printf("Detected SET, Resent UA, going to try and read new Start Pack\n");
				  readStart=FALSE;
				  break;
			    }

			    switch(response.arr[2])
				{
					case 0x00:
						//readFilePAcketInfo(pr);
						//if no errors send RR0 ->FILE  INFO PACK Reading successful
						writeBytes(fd,response.arr);
						break;
					case 0x40:
						//readFilePAcketInfo(pr);
						//if no errors send RR0 ->FILE INFO PACK Reading successful
						writeBytes(fd,response.arr);
						break;
					case 0x01:
						//sent REJ
						writeBytes(fd,response.arr);
						break;
				}
			}
		}
	}

}


int main(int argc, char** argv)
{
    int fd;
    struct termios oldtio,newtio;



    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(-1);
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

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
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

    printf("New termios structure set\n");
	//llopen(fd,0);
		//llread(fd);
		/*
		//testing code for destuffing functions
		DataPack testpack;
		testpack.size=10;
		testpack.arr=malloc(testpack.size);
		testpack.arr[0]=0x00;
		testpack.arr[1]=0x00;
		testpack.arr[2]=0x00;
		testpack.arr[3]=0x00;
		testpack.arr[4]=0x40;
		testpack.arr[5]=0x7D;
		testpack.arr[6]=0x5D;
		testpack.arr[7]=0x7D;
		testpack.arr[8]=0x5e	;
		testpack.arr[9]=0x7E;
		printArray(testpack.arr,testpack.size);
		testpack=destuffPack(testpack);
		printArray(testpack.arr,testpack.size);
		//end of testing code for destuffing functions
		*/

    sleep(2);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
