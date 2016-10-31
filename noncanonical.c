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
#include "utils.h"



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

	char set[5]={0x7E,0x03,0x03,0x03^0x03,0x7E};
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
			return 0x7E;
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
 char ua[5]={0x7E,0x03,0x07,0x03^0x07,0x7E};
 char readchar[2];
 int counter = 0;
 if(type==0){
	 while (STOP==FALSE) {       /* loop for input */

	  readchar[0]=readSupervision(fd,counter);
	  printf("0x%02x \n",(unsigned char)readchar[0]);
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

DataPack makeErrorPack()
{
	DataPack errpack;
	errpack.size=1;
	errpack.arr=malloc(errpack.size);
	errpack.arr[0]=-1;
	return errpack;
}

int validateBCC2(DataPack dataPacket,unsigned char BCC2){
	int i;
	unsigned char makeBCC2;
	makeBCC2 = dataPacket.arr[0]^dataPacket.arr[1];

	for (i = 2; i < dataPacket.size;i++)
	{
		makeBCC2 = makeBCC2^dataPacket.arr[i];
	}

	printf("makeBCC2:0x%02x BCC2:0x%02x\n",makeBCC2,BCC2);
	if(BCC2==makeBCC2)
		return 0;
	return -1;
}

DataPack destuffPack(DataPack todestuff)
{
	printf("BEFORE DESTUFF size:%d  \n",todestuff.size);
	//TODO make this more accessible to other functions
	char* dbuf=malloc(todestuff.size-6);
	DataPack dataPacket;
	// counter for finding all bytes to destuff, starts on 4 because from 0 to 3 is the header
	//j is a counter to put bytes on dbuf;
	int i=4;

	int j=0;

	//Finding content that needs destuffing
	while(i<todestuff.size-2)
	{
		
		if(todestuff.arr[i] == 0x7E){
					printf("Stuffing failed, found 0x7E before final position of packet\n");
					return makeErrorPack();
		}

		//no flags
		if(todestuff.arr[i] != 0x7D){

		 dbuf[j] = todestuff.arr[i];
		 i++;
		 j++;
		 continue;
		}

		if(todestuff.arr[i] == 0x7D){

			if(todestuff.arr[i+1] == 0x5E){
				dbuf[j] = 0x7E;
				i = i+2;
				j++;
				continue;
		 	}

			if(todestuff.arr[i+1] == 0x5d){
				dbuf[j] = 0x7D;
				i = i+2;
				j++;
				continue;
			}
			printf("Stuffing failed, found unstuffed 0x7D\n");
			return makeErrorPack();
		}

	}

	if(todestuff.arr[todestuff.size-1]!=0x7E){
		printf("Invalid Packet, found 0x%02x at final position of packet, should be 0x7E\n",(unsigned char)todestuff.arr[todestuff.size-1]);
		return makeErrorPack();
	}

  dataPacket.size=j;
  dataPacket.arr=dbuf;

	if(validateBCC2(dataPacket,(unsigned char)todestuff.arr[todestuff.size-2])==-1){
		printf("BCC2 doesn't match with BCC2 of received contents, please resend Packet\n");
		return makeErrorPack();
	}

	printf("AFTER DESTUFF Size:%d \n",dataPacket.size);
	return dataPacket;
}



ResponseArray readInfPackHeader(int fd, char* buf){

	//Verifying that the header of the package is correct
	ResponseArray response;
	char c1alt;
	char REJ[5]={0x7E,0x03,0x01,0x03^0x01,0x7E};
	char restartERR2[5]={ERR2,ERR2,ERR2,ERR2,ERR2};
	printf("PRINTING JUST THE HEADER\n");
	printArray(buf,4);
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

	char REJ[5]={0x7E,0x03,0x01,0x03^0x01,0x7E};

	tempI[0]=startPacket[6];
	tempI[1]=startPacket[5];
	tempI[2]=startPacket[4];
	tempI[3]=startPacket[3];

	int currentI=6;
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




	printf("FILE NAME IS IS %s \n",file.arr);

	return res;
}

DataPack getPacket(int fd,int wantedsize){
	DataPack sp;
	sp.size=wantedsize;
	sp.arr=malloc(sp.size);
	int res=-1;
	int counter=0;
	int first7E = FALSE;

	while(counter<wantedsize)
	{
		res = read(fd,&sp.arr[counter],1);
		if(counter <5)
			printf("0x%02x\n",sp.arr[counter]);
		if(res==-1)
		{
			printf("ERROR READING: QUITTING\n");
			exit(-1);
		}
					// 	printf("CURRENT CHARACTER:0x%02x\n",(unsigned char)readchar[counter]);


		if(first7E==TRUE){
			if(sp.arr[counter]==0x7E)
			{
				printf("FOUND 2ND 7E %d\n",counter);
				sp.size=counter+1;
				sp.arr = realloc(sp.arr,sp.size);
				printf("SIZEOF sp.arr AFTER REALLOC %d\n",sp.size);
				break;
			}
		}

		if(sp.arr[counter]==0x7E)
		{
			first7E=TRUE;
		}
		counter++;
	}

	return sp;
}

void validateStartPack(int fd){

	DataPack sp=getPacket(fd,50);

	printArray(sp.arr,sp.size);


	ResponseArray response =readInfPackHeader(fd,sp.arr);

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
		sp = destuffPack(sp);
		if(sp.arr[0]==-1){
			readStart=FALSE;
			return;
		}
		response = readStartPacketInfo(sp.arr,response);
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

  
void writeFileInfo(DataPack data){	
	printf("Writing to file %s -> %d bytes\n",file.arr,data.size);
	fwrite(data.arr,1,data.size,fp);
}

void openFile()
{
	fp=fopen(file.arr,"wb");
	if(fp==NULL)
	{
		printf("FILE POINTER NULL, bye bye \n");
		exit(-1);
	}
}

  
void llread(int fd)
{


	//TRYING TO READ JUST THE START PACKET
	while(readStart == FALSE)
	{

		validateStartPack(fd);

		if(readStart==TRUE)
		{	
			openFile();
			int SizeRead=0;
		 	
		 	while (SizeRead<file.fileSize)
			{
				DataPack filepacket;
				while(packetValidated==FALSE)
				{
					filepacket=getPacket(fd,PACKET_SIZE+500);
					//read first 4 bytes to readchar, send readchar to readInfpacketHeader
					ResponseArray response =readInfPackHeader(fd,filepacket.arr);

					if(response.arr[0]==ERR2)
					{
					  printf("Detected SET, Resent UA, going to try and read new Start Pack\n");
					  SizeRead=0;
					  readStart=FALSE;
					  break;
				    }

				    switch(response.arr[2])
					{
						case 0x00:
							printf("Validated File Packet Header, gotta break it down now\n");
							filepacket = destuffPack(filepacket);
							if(filepacket.arr[0]==-1){
								packetValidated=FALSE;
								continue;
							}
							printf("Sending RR0\n");
							printArray(response.arr,5);
							writeBytes(fd,response.arr);
							packetValidated=TRUE;
							break;
						case 0x40:
							printf("Validated File Packet Header, gotta break it down now\n");
							filepacket = destuffPack(filepacket);
							if(filepacket.arr[0]==-1){
								packetValidated=FALSE;
								continue;
							}
						    printf("Sending RR1\n");
						    printArray(response.arr,5);
							writeBytes(fd,response.arr);
							packetValidated=TRUE;
							break;
						case 0x01:
							//sent REJ
							printf("Sending REJ\n");
							writeBytes(fd,response.arr);
							packetValidated=FALSE;
							continue;
					}
				}				
				

				if(readStart==FALSE){
					printf("starting from beginning\n");
					break;
				}
				
				SizeRead+=filepacket.size;
				writeFileInfo(filepacket);
				packetValidated=FALSE;
				if(SizeRead>=filepacket.size)
					readFile=TRUE;
			}
			printf("readStart %d \n",readStart);
			printf("readFile %d \n",readFile);
		}
		
	}
	fclose(fp);

	printf("llread ended with success");
	return;

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
 
  	    llopen(fd,0);
		llread(fd);
		/*
		//testing code for destuffing functions
		DataPack testpack;
		testpack.size=8;
		testpack.arr=malloc(testpack.size);
		testpack.arr[0]=0x00;
		testpack.arr[1]=0x00;
		testpack.arr[2]=0x00;
		testpack.arr[3]=0x00;
		testpack.arr[4]=0x03;
		testpack.arr[5]=0x01;
		testpack.arr[6]=0x02;
		testpack.arr[7]=0x7E;

		printArray(testpack.arr,testpack.size);
		testpack=destuffPack(testpack);

		if(testpack.arr[0]==-1)
		exit(-1);
		printArray(testpack.arr,testpack.size);
		//end of testing code for destuffing functions
		*/

    sleep(2);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
