
#include "geturl.h"

int parsePath(char * fullPath, url_t *url)
{
	int i = 0, willHaveLogin = 0, counter = 0;
	
	char model[STR_SIZE];
	sprintf (model, "ftp://");

	//Check if Path starts with "ftp://"
	while (i <= 5)
	{
		if (fullPath[i] != model[i])
		{
			printf("Wrong input\n");
			return 1;
		}
		i++;
	}

	counter = 6;

	//Check if path will recquire username and password
	if (fullPath[counter] == '[')
		willHaveLogin = TRUE;
	else
		willHaveLogin = FALSE;

	int ctrl = 0;

	//This if statement makes sure that user and password are correctly stored in the struct
	if (willHaveLogin)
	{
		counter++;
		ctrl = 0;
		char * tempUser = malloc(255);

		//Reading username
		while (fullPath[counter] != ':')
		{
			tempUser[ctrl] = fullPath[counter];
			counter++;
			ctrl++;
		}
		url->username=malloc(ctrl);
		strncpy(url->username,tempUser,ctrl);
		free(tempUser);
		counter++;

		ctrl  =0;
		char * tempPass = malloc(255);

		//Reading password
		while (fullPath[counter] != '@')
		{
			tempPass[ctrl] = fullPath[counter];
			counter++;
			ctrl++;
		}
		url->password=malloc(ctrl);
		strncpy(url->password,tempPass,ctrl);
		free(tempPass);

		if (fullPath[counter] != '@' )
		{
			printf("Bad input - missing '@'.\n");
			return 1;
		}

		counter++;

		if (fullPath[counter] != ']' )
		{
			printf("Bad input - missing ']'.\n");
			return 1;
		}
			counter++;
	}
	else
	{
		//If there's no username and password, these fields become 'NULL'
		url->username = malloc(sizeof("NULL"));
		strncpy(url->username,"NULL",sizeof("NULL"));

		url->password = malloc(sizeof("NULL"));
		strncpy(url->password,"NULL",sizeof("NULL"));
	}

	ctrl = 0;
	char * tempHost = malloc(255);
	while (fullPath[counter] != '/')
	{
		tempHost[ctrl] = fullPath[counter];
		counter++;
		ctrl++;
	}
	url->host=malloc(ctrl+1);
	strncpy(url->host,tempHost,ctrl+1);
	free(tempHost);

	//counter++;
	ctrl = 0;
	char * tempPath = malloc(255);
	int gettingMode  = 0;
	char* filename;
	int fileCounter = 0;
	while (fullPath[counter] != '\0')
	{
		tempPath[ctrl] = fullPath[counter];
		ctrl++;
		counter++;
		filename[fileCounter] = tempPath[ctrl];
		fileCounter++;
		if(tempPath[ctrl] = '/' &&  fullPath[counter +1] != '\0'){
			gettingMode = 1;
			memset(&filename, 0, sizeof(filename));
			fileCounter = 0;
		}
		
		
	}

	url->path=malloc(ctrl);
	strncpy(url->path,tempPath,ctrl);
	strncpy(url->filename,filename,fileCounter);
	free(tempHost);
	return 0;
}


int getIpByHost(url_t* url) {
	struct hostent *h;

            

        if ((h=gethostbyname(url->host)) == NULL) {  
            herror("gethostbyname");
            exit(1);
        }

        //printf("Host name  : %s\n", h->h_name);
        char* ip = inet_ntoa(*((struct in_addr *)h->h_addr));
		strncpy(url->ip, ip, strlen(ip));
        return 0;
}
