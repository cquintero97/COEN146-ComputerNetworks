//	Christian Quinter
//	COEN 146L
//	udpserver.c
//
//	This file uses UDP connection to receive a filename and data from a client
//	and then writes the data onto a file with a name it received from the client.

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#define BUFF_SIZE 10

typedef struct 
{
	int sequence_ack;
	int length;
} HEADER;

typedef struct 
{
	HEADER header;
	char data[BUFF_SIZE];
} PACKET;

PACKET packet;
PACKET *p = &packet;
PACKET received;
PACKET *r = &received;

int sock;
int nBytes;
int seqnum = 0;
struct sockaddr_in serverAddr, clientAddr;
struct sockaddr_storage serverStorage;
socklen_t addr_size, client_addr_size;

void receiveMessage();
void sendMessage(int seq);

/********************
 * main
 ********************/
int main (int argc, char *argv[])
{
	char filename[BUFF_SIZE];

	// Configure address
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons ((short)atoi (argv[1]));
	serverAddr.sin_addr.s_addr = htonl (INADDR_ANY);
	memset ((char *)serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));  
	addr_size = sizeof (serverStorage);

	// Create socket
	if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf ("socket error\n");
		return 1;
	}

	// Bind
	if (bind (sock, (struct sockaddr *)&serverAddr, sizeof (serverAddr)) != 0)
	{
		printf ("bind error\n");
		return 1;
	}

	// This will Receive filename outputted by client
	receiveMessage();
	strcpy(filename, r->data);
 
	FILE *fpo = fopen(filename, "w");

	// This while loop will receive data from the client
	while (1) 
	{
		receiveMessage();
    
		// If the packet length is 0 then it will close the output file and return
		if (r->header.length == 0) 
		{
			fclose(fpo);
			return 0;
		}
    
		// start writing data received to fpo
		fwrite(r->data, 1, r->header.length, fpo);
	}
  
	return 0;
}

void receiveMessage() 
{
	int mystate = 0; 
	while (mystate == 0) 
	{
		// Receive data then send ack
		nBytes = recvfrom(sock, r, sizeof(packet), 0, (struct sockaddr *)&serverAddr, &addr_size);
		sendMessage(r->header.sequence_ack);

		if (r->header.sequence_ack == seqnum)
		mystate = 1;
	}
  
	// flip seqnum
	if (seqnum == 0)
		seqnum = 1;
	else
		seqnum = 0;
}

void sendMessage(int seq) 
{
	p->header.sequence_ack = seq;
	p->header.length = 0;
  
	// this will send the data to client
	sendto(sock, p, sizeof(packet), 0, (struct sockaddr *)&serverAddr, addr_size);
}
