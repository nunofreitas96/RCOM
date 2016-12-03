
//************************************************************************************************************
//TODO create ftp struct - data e control se passarem a ser partes de uma struct serão mias faceis de controlar 
//**************************************************************************************************************
static int connectSocket(const char* ip, int port) {
	int sockfd;
	struct sockaddr_in server_addr;

	// server address handling
	bzero((char*) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip); /*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port); /*server TCP port must be network byte ordered */

	// open an TCP socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket()");
		return -1;
	}

	// connect to the server
	if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr))
			< 0) {
		perror("connect()");
		return -1;
	}

	return sockfd;
}

int connectFTP( const char* ip, int port, int control, int* data){
	char rd[1024];

	data = 0;

	if ((control = connectSocket(ip, port)) < 0) {
		printf("Socket cannot be connected.\n");
		return 1;
	}

	

	return 0;

}

int loginFTP(int control, const char* user, const char* password){
	char userTest[1024];
	char passTest[1024];

	sprintf(userTest, "user %s\n", user);
	sprintf(passTest, "pass %s\n", password)


	if(sendFromFTP(control, userTest, sizeof(userTest))) {
		printf("Sending to FTP failed.\n");
		return 1;
	}

	if(readFromFTP(control, userTest, sizeof(userTest))){
		printf("Read from FTP failed.\n");
		return 1;
	}

	if(sendFromFTP(control, passTest, sizeof(passTest))){
		printf("Sending to FTP failed.\n");
		return 1;
	}


	if(readFromFTP(control, passTest, sizeof(passTest))){
		printf("Read from FTP failed.\n");
		return 1;
	}
}

int changeDirFTP(int control, const char* path){
	char currPath[1024];

	sprintf(currPath, "XWD %s\n", path);

	if(sendFromFTP(control, currPath, sizeof(currPath))){
		printf("Sending to FTP failed.\n");
		return 1;
	}


	if(readFromFTP(control, currPath, sizeof(currPath))){
		printf("Read from FTP failed.\n");
		return 1;
	}

	return O;
}



int passiveModeFTP(int control){
	char passive[1024];
	char passiveIp[1024];
	sprintf(passive, "PASV\n");

	if(sendFromFTP(control, retr, sizeof(retr))){
		printf("Sending to FTP failed.\n");
		return 1;
	}


	if(readFromFTP(control, retr, sizeof(retr))){
		printf("Read from FTP failed.\n");
		return 1;
	}

	int ip1,ip2,ip3,ip4;
	int port1, port2;
	if((sscanf(pasv,"227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip1,&ip2,&ip3,&ip4,&ip4,&port1,&port2)) < 0){
		printf("Response is wrong.");
		return 1;
	}

	int port = port1*256 + port2;

	if((data = connectSocket(passiveIp,port)) < 0){
		printf("Passive mode cannot be entered.")
		return 1;
	}

	return 0;
 
}

int copyFileFTP(control, const char* filename){
	char retr[1024];

	sprintf(retr, "RETR %s\n", filename);

	if(sendFromFTP(control, retr, sizeof(retr))){
		printf("Sending to FTP failed.\n");
		return 1;
	}


	if(readFromFTP(control, retr, sizeof(retr))){
		printf("Read from FTP failed.\n");
		return 1;
	}

	return O;

}

int downloadFileFTP(int* control,int* data, const char* data){
	FILE* file;
	int nbytes;

	if (!(file = fopen(filename, "w"))) {
		printf("File cannot be opened.\n");
		return 1;
	}

	char buf[1024];

	while((bytes = read(data, buf, sizeof(buf))) ){
		if (bytes < 0) {
			printf("Data socket sent nothing.\n");
			return 1;
		}

		if((bytes = fwrite(buf,bytes, 1 , file)) < 0 ){
			printf("Write failed.\n");
			return 1;
		}
	}

	fclose(file);
	close(data);

	return 0;
}

int disconnectFromFTP(int control){
	char disc[1024];

	sprintf(disc, "QUIT\n");

	if(ftpSend(control, disc, strlen(disc))){
		printf("Sending to FTP failed.\n");
		return 1;
	}

	if(readFromFTP(control, retr, sizeof(retr))){
		printf("Read from FTP failed.\n");
		return 1;
	}

	return 0;
}

int sendToFTP(int control, const char* str, size_t size){
	int bytes; 

	if((bytes= write(control,str,size))<= 0){
		//mensagem de erro
	}

	printf("Bytes written: %d ; Info written: %d", bytes, str);

	return 0;
}


int readFromFTP(int control, const char* str, size_t size){
	FILE fp = fdopen(control, "r");


	do {
		memset(str, 0, size);
		str = fgets(str, size, fp);
		printf("%s", str);
	} while (!('1' <= str[0] && str[0] <= '5') || str[3] != ' ');

	return 0;
}
