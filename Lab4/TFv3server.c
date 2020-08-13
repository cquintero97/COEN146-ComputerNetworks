// Christian Quintero
// COEN 146L 
// TFv3server.c
// File with server responsible for initializing connection and receiving output file
// name. Also responsible for sending acknowledgment to client in case of loss or bit error.

#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#define SIZE 10

typedef struct 
{
    int sequence_ack;
    int len;
    int checksum;
    int fin;
} HEADER;

typedef struct 
{
    HEADER header;
    char data[SIZE];
} PACKET;

PACKET packet;
PACKET *p = &packet;
PACKET received;
PACKET *r = &received;
int seq_num = 0;

int sock, nBytes;
struct sockaddr_in serverAddr, clientAddr;
struct sockaddr_storage serverStorage;
socklen_t addr_size, client_addr_size;

int checks(PACKET *packet);
void receiveMessage();
void sendMessage(int seq);

/********************
 * main
 ********************/
int main(int argc, char *argv[]) 
{
    srand(time(NULL)); 
    char filename[10];

    // Configure address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons((short) atoi(argv[1]));
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset((char *) serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    addr_size = sizeof(serverStorage);

    // Create socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
        printf("socket error\n");
        return 1;
    }

    // Bind
    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0) 
	{
        printf("bind error\n");
        return 1;
    }

    // Receive output filename
    receiveMessage();
    strcpy(filename, r->data);
	
    // Open output file in order to write into
    FILE *fpo = fopen(filename, "w");

    // Receive data from clients
    while (1) 
	{
        receiveMessage();
        if (r->header.len == 0) 
		{
            fclose(fpo);
            return 0;
        }

        // Write data to the output file
        fwrite(r->data, 1, r->header.len, fpo);
    }

    return 0;
}

// Calculates checksum
int checks(PACKET *packet) 
{
    packet -> header.checksum = 0;
    char checksum = 0;
    char *p = (char *)packet;
    int i;
    for (i = 0; i < packet->header.len; i++) 
	{
        checksum ^= *p;
        p++;
    }
    return (int) checksum;
}

void sendMessage(int seq) 
{
    // Set packet details
    p->header.sequence_ack = seq;
    p->header.len = 0;
    int checksum = checks(p);
    p->header.checksum = checksum;
    // Send data
    sendto(sock, p, sizeof(packet), 0, (struct sockaddr *)&serverAddr, addr_size);
}

void receiveMessage() 
{
    int mystate = 0;
    int checksum;

    while (mystate == 0) 
	{
        nBytes = recvfrom(sock, r, sizeof(packet), 0, (struct sockaddr *)&serverAddr, &addr_size);
        
        // random decision to send ack message or not for error testing
	if (rand() % 100 > 20)
            sendMessage(r->header.sequence_ack);
	else 
	  printf("No ack sent!\n");

        int Rchecksum = r->header.checksum;
        // random decision to send the right checksum or not for error testing
        if (rand() % 100 > 2)
            checksum = checks(r);
        else
            checksum = 0;

        if (checksum == Rchecksum && r->header.sequence_ack == seq_num) 
		{
            mystate = 1;
			// Flip
			if (seq_num == 0)
				seq_num = 1;
			else
				seq_num = 0;
		}
    }
}
