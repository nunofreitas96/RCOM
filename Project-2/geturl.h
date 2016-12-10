#ifndef GETURL_H
#define GETURL_H

#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#define STR_SIZE 256
#define TRUE 1
#define FALSE 0

typedef	struct 
{	
	char* host;
	char* path;
	char* username;
	char* password;
	char* ip;
	char* filename;
	
}url_t;

int parsePath(char * fullPath, url_t *url);

int getIpByHost(url_t* url);

#endif