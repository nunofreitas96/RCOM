#include <stdio.h>
#include "geturl.c"
#include "ftp.c"

static void printUsage(char* argv0);

int main(int argc, char** argv){
	
	if(argc != 2){
		perror("Incorrect number of arguments");
		exit(0);
		
	}
	
	
	url_t url;
	
	if(parsePah(argv[1] , &url)){
		perror("Failed on parsing path");
		exit(0);
	}
	
	if(getIpByHost(&url)){
		perror("Failed on obtaining ID");
		exit(0);
	}
	
	int port = 21;
	ftpSockets ftp;
	
	if(connectFTP(&url.ip, &port,ftp)){
		perror("Failed on COnnecting to FTP");
		exit(0);
	}
	
	if(loginFTP(&url.user,&url.password, ftp)){
		perror("Failed on COnnecting to FTP");
		exit(0);
	}
	
	if(changeDirFTP(&url.path,ftp)){
		perror("Failed on COnnecting to FTP");
		exit(0);
	}
	
	if(passiveModeFTP(ftp)){
		perror("Failed to enter passive mode.");
		exit(0);
	}
	
	
	if(copyFileFTP(url.filename,ftp)){
		perror("Failed to copy file.");
		exit(0);
	}
	
	if(downloadFileFTP(url.filename,ftp)){
		perror("Failed to download file");
		exit(0);
	}
	
	if(disconnectFromFTP(ftp)){
		perror("Failed to disconnect from FTP");
		exit(0);
	}
	
	return 0;
}