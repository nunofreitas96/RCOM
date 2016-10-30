/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
	char arr[4];
} ResponseArray;

typedef struct{
	char* arr;
	int size;
} StartPack;

typedef struct{
	char* arr;
	int namelength;
	int fileSize;
} FileData;


//char* file_name;
FileData file;
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
			printf("SUCCESS\n");
			return 0x76;
		}
		else printf("ERROR\n");
		return ERR;
	case 1:
		if(buf[0]==set[1]){
			return 0x03;
		}
		else printf("ERROR\n");
		return ERR;
	case 2:
		if(buf[0]==set[2]){
			printf("SUCCESS\n");
			return 0x03;
		}
		else printf("ERROR\n");
		return ERR;
	case 3:
		if(buf[0]==set[3]){
			printf("SUCCESS\n");
			return buf[0];
		}
		else printf("ERROR\n");
		return ERR2;
	case 4:
		if(buf[0]==set[4]){
			printf("SUCCESS\n");
			return 0X7E;
		}
		else printf("ERROR\n");
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

StartPack destuffPack(int fd,char* buf,size_t length)
{
	//Buffer whose content will be destuffed
	//TODO make this more accessible to other functions
	char dbuf[length];
	StartPack startPack;
	// counter for finding all bytes to destuff, starts on 4 because from 0 to 3 is the header
	//j is a counter to put bytes on dbuf;
	int i =4;
	int j =0;

	//Finding content that needs destuffing
	while(TRUE){
		if(buf[i] == 0x7E){

			break;
		}
		//TODO verificar se eu percebi o destuffing corretamente
		//counter measure de flags
		if(buf[i] == 0x7d){

			if(buf[i+1] == 0x5e){
				dbuf[j] = 0x7E;
				i = i+2;
				if(i >= strlen(buf)){
					printf("esse stuffing ta mal, oh boi");
					//return ERR;
				}

			}
			else if(buf[i+1] == 0x5f){
				dbuf[j] = 0x7D;
				i = i+2;
					if(i >= strlen(buf)){
					printf("esse stuffing ta mal, oh boi");
					//return ERR;
				}
			}
			else{
				//TODO mudar antes de entrega a mensagem de erro
				printf("esse stuffing ta mal, oh boi");
				//return ERR;
			}
		}
		//no flags
		else{
		dbuf[j] = buf[i];
		i++;
		if(i >= strlen(buf)){
					printf("esse stuffing ta mal, oh boi");
					//return ERR;
				}
		}

	j++;


	}

	memcpy(startPack.arr,dbuf,strlen(dbuf));
	printf("Received header with no errors, printed RR\n");

	return startPack;



}

void printArray(char* arr,size_t length){

	int index;
	for( index = 0; index < length; index++){
			printf( "0x%X\n", (unsigned char)arr[index] );
	}
}

ResponseArray readInfPackHeader(int fd, char* buf){

	//TODO Verificar se está tudo correto

	//Verifying that the header of the package is correct
	ResponseArray response;
	char c1alt;
	char REJ[4]={0x7E,0x03,0x01,0x03^0x01};
	char restartERR2[4]={ERR2,ERR2,ERR2,ERR2};


	//Verifying starting flag
	if(buf[0] != 0x7E){
		printf("first byte isn't flag error \n");
		memcpy(response.arr,REJ,4);
		return response;
	}

	//Verifying A
	if(buf[1] != 0x03){
		printf("read error in (A) \n");
		memcpy(response.arr,REJ,4);
		return response;
	}

	//Verifying C1
	if(buf[2] != 0x00 && buf[2] != 0x40){
		printf("read error in (C)");
		memcpy(response.arr,REJ,4);
		return response;
	}
	else if(buf[2]== 0x03){
		if(buf[3]==0x00){
			llopen(fd,1);
			memcpy(response.arr,restartERR2,4);
			return response;
		}

		else{
			printf("Invalid information packet\n");
		}

	}
	//Verifying BCC1
	if((buf[1]^buf[2]) != buf[3]){
		printf("A^C is not equal to BCC1 error");
	    memcpy(response.arr,REJ,4);
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
	char RR[4] = {0x7E,0x03,c1alt,0x03^c1alt};
	memcpy(response.arr,RR,4);
	printf("Received header with no errors, printed RR\n");

	return response;
}


void readStartPacketInfo(char * startPacket){

char tempI[5];
char temp[50];
int offset = 4;

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




	int fileNameSize = startPacket[currentI+2];
	printf("FILE NAME SIZE IS IS %d \n",fileNameSize);
	file.arr=malloc(fileNameSize+1);
	int i =0;

	while( i < fileNameSize){
		file.arr[i] = startPacket[currentI+3+i];
		i++;
	}
	printf("FILE NAME IS IS %s \n",file.arr);
}



void validateStartPack(int fd){
//if ( p != (tmp = realloc(p, required_size)) )
//	memcpy(tmp, p = tmp, required_size);
 	StartPack sp;
  sp.size=50;
	sp.arr=malloc(sp.size);


	int res=-1;
  int counter=0;

   /*while(counter<26)
	 {
			res = read(fd,&readchar[counter],1);
			if(res==-1){
				printf("ERROR READING: QUITTING\n");
			}
			counter++;
	 }*/
	 int first7E = FALSE;
	 while(counter<40)
	 {
	 	res = read(fd,&sp.arr[counter],1);

		if(res==-1){
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
	printf("reading the response that I received from readInfPackHeader\n");
	printArray(response.arr,4);
	printf("END OF RESPONSE READ2323:\n");

	if(response.arr[0]==ERR2)
	{
		printf("Detected SET, Resent UA, going to try and read new Start Pack\n");
		readStart=FALSE;
		return;
	}

	switch(sp.arr[2])
	{
		case 0x00:
		printf("Validated Starter Packet Header, gotta break it down now\n");
		readStartPacketInfo(sp.arr);
		//if no errors send RR0 ->START PACK Reading successful, ready to begin reading actual file
		//StartPack startPack = destuffPack(fd, sp.arr, strlen(sp.arr)-4);
		writeBytes(fd,response.arr);
		readStart=TRUE;
		break;

		case 0x01:
		printf("Rejecting invalid Starter Packet, try again \n");
		writeBytes(fd,response.arr);
		readStart=FALSE;
		break;
	 }
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
		 	 while (readFile==TRUE)
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
		llread(fd);


    sleep(2);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
