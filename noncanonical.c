/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;
void writeBytes(int fd, char* message)
{
  	
	printf("SendBytes Initialized\n");
    int size=strlen(message);
	int sent = 0;

    while( (sent = write(fd,message,size+1)) < size ){
        
        size -= sent;
    }
	
}
void readBytes(int fd)
{
   	char collectedString[255]; 	
	char buf[2];    
	int counter=0,res=0;
    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf,1);   /* returns after 1 chars have been input */
      buf[1]='\0';	
      collectedString[counter]=buf[0]; 
     // printf("%d:%s\n",res, buf);
      
      if (buf[0]=='\0'){ 
	  collectedString[counter]=buf[0]; 
	  STOP=TRUE;
      }
      counter++;
    }
    printf("end result:%s\n",collectedString);
	writeBytes(fd,collectedString);
}

int main(int argc, char** argv)
{
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
	readBytes(fd);
 	

    
   // write(fd,collectedString,strlen(collectedString)+1); 

 /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião 
  */
    pause(2);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
