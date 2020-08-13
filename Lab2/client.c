// Christian Quintero
// FTPv1 client.c
// Lab 2
//
// Client file that sends output file name, reads input file, and sends 
// the data read from input file to the server.

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#define BUFF_SIZE 10

int main (int, char *[]);


/********************
 * main
 ********************/
int main (int argc, char *argv[])
{
	// initialize values
	int sockfd = 0, n = 0;
	char buff[BUFF_SIZE];
	struct sockaddr_in serv_addr; 

	// checks for appropriate number of arguemnts
	if (argc != 5)
	{
		printf ("incorrect number of arguements\n");
		return 1;
	} 

	// set up
	memset (buff, '0', sizeof (buff));
	memset (&serv_addr, '0', sizeof (serv_addr)); 

	// open socket
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf ("Error : Could not create socket \n");
		return 1;
	} 

	// set address
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons (atoi(argv[2])); 

	if (inet_pton (AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
	{
		printf ("inet_pton error occured\n");
		return 1;
	} 

	// connect
	if (connect (sockfd, (struct sockaddr *)&serv_addr, sizeof (serv_addr)) < 0)
	{
		printf ("Error : Connect Failed \n");
		return 1;
	} 
	
	// send name of output file to server	
	write(sockfd, argv[4], (strlen(argv[4])+1));
	
	// open the input file for reading
	FILE *fpr = fopen(argv[3], "r");
	while ((n = fread(buff, 1, 10, fpr)) > 0)
	{
		// send the data in file in chunks of at most 10 to the server
		write(sockfd, buff, n);
	}
	
	// close the file & socket connection
	fclose (fpr);
	close (sockfd);

	return 0;
}
