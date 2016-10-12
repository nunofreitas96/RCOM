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

volatile int STOP=FALSE;

//Funções GUIA1
void writeBytes(int fd, char* message)
{
  	
	printf("SendBytes Initialized\n");
    int size=strlen(message);
	int sent = 0;

    while( (sent = write(fd,message,size+1)) < size ){
        size -= sent;
    }
	
}


char * readBytes(int fd)
{
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
	return 0x36;
	}
	    
	switch(counter){
	case 0:
		if(buf[0]==set[0]){
			printf("SUCCESS\n");
			return 0x76;		
		}
		else printf("ERROR\n");
		return 0x36;
	case 1:
		if(buf[0]==set[1]){

			printf("SUCCESS\n");
			return 0x03;		
		}
		else printf("ERROR\n");
		return 0x36;
	case 2:
		if(buf[0]==set[2]){
			printf("SUCCESS\n");
			return 0x03;		
		}
		else printf("ERROR\n");
		return 0x36;
	case 3:
		if(buf[0]==set[3]){
			printf("SUCCESS\n");
			return buf[0];		
		}
		else printf("ERROR\n");
		return 0x35;
	case 4:
		if(buf[0]==set[4]){
			printf("SUCCESS\n");
			return 0X7E;		
		}
		else printf("ERROR\n");
		return 0x36;
 	default:
		return 0x36;
	}	
}

void llopen(int fd){
 char ua[5]={0x7E,0x03,0x03,0x01,0x7E};
 char readchar[2];
 int counter = 0;

 while (STOP==FALSE) {       /* loop for input */
 
  readchar[0]=readSupervision(fd,counter);
  printf("%c \n",readchar[0]);
  readchar[1]='\0';	
  counter++;

  if(readchar[0]==0x36){
  counter=0;
  }
   
  if(readchar[0]==0x35){
  counter=-1;
  }
  
  if (counter==5){ 
	 STOP=TRUE;
  }
  
 }
	printf("Sending UA...\n");
    writeBytes(fd,ua);
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

    printf("New termios structure set\n");
	llopen(fd);
 	

    sleep(2);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
