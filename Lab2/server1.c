// Christian Quintero 
// COEN 146l Thur. 9:10am
// lab2
// server1.c
//
// This is the server file that receives the file name and data from client 
// to write onto an output file.

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>

#define BUFF_SIZE 10

int main (int, char *[]); 


/*********************
 * main
 *********************/
int main (int argc, char *argv[])
{
	// initialize variables
	int n;
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr; 
	char buff[BUFF_SIZE];

	// check if port number is missing
	if (argc != 2)
	{
		printf("missing port number.\n");
		return 0;
	}

	// set up
	memset (&serv_addr, '0', sizeof (serv_addr));
	memset (buff, '0', sizeof (buff)); 

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	serv_addr.sin_port = htons (atoi(argv[1])); 

	// create socket, bind, and listen
	listenfd = socket (AF_INET, SOCK_STREAM, 0);
	bind (listenfd, (struct sockaddr*)&serv_addr, sizeof (serv_addr)); 
	listen (listenfd, 10); 
	
	// Declare file pointer and file name of output file
	FILE *fpc;
	char fname[1000];
	
	// accept and interact
	while (1)
	{
		connfd = accept (listenfd, (struct sockaddr*)NULL, NULL); 
		// takes in created file name and stores in string
		if (read(connfd, fname, sizeof(fname)) > 0);
		{
			// open file for writing
			fpc = fopen(fname, "w");
			// write in file by buffer size chunks
			while ((n = read(connfd, buff, sizeof(buff))) > 0)
			{
				fwrite(buff, 1, n, fpc);
			}
		
		// close file and socket connection
		fclose (fpc);
		close (connfd);
		return 0;
		}
	}
}
