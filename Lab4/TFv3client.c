// Christian Quintero
// COEN 146L 
// TFv3client.c
// File with server responsible for connecting to server and sending data 
// to server, along with checksum to check for bit error. Also has condition
// to check for timeout and resending of data.

#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <string.h>
#define SIZE 10

typedef struct 
{
    int sequence_ack;
    int length;
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

int nBytes, sock;
struct sockaddr_in serverAddr;
socklen_t addr_size;

struct timeval tv;
int rv;
int times_sent_zero = 0;

int checks(PACKET *p);
void sendMessage(int seq, char *data, int n);
void receiveMessage();

/***********
 *  main
 ***********/
int main(int argc, char *argv[]) 
{
	char buffer[10];

    srand(time(NULL));

    // Configure address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &serverAddr.sin_addr.s_addr);
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    addr_size = sizeof serverAddr;

    // Create UDP socket 
    sock = socket(PF_INET, SOCK_DGRAM, 0);

    // send output filename and check for successful reception
    sendMessage(seq_num, argv[4], strlen(argv[4]) + 1);
    receiveMessage();

    // Open input file to read
    FILE *fpi = fopen(argv[3], "r");

    // This will read data from input file then send data 
    while (nBytes = fread(buffer, 1, 10, fpi)) 
	{
        if (rand() % 100 > 2) 
		{ // This randomizes the decision to send data, to test error cases
            
            sendMessage(seq_num, buffer, nBytes);
            if (rv == 1) 
			{ // Then there is data to be received
                receiveMessage();
            }
        }
    }
    // Close file
    fclose(fpi);

    // this will send a final and empty message to signal end of data
    sendMessage(seq_num, "", 0);

    // Close socket connection
    close(sock);
    return 0;
}

// function to calculate checksum
int checks(PACKET *packet) 
{
    packet->header.checksum = 0;
    char checksum = 0;
    char *p = (char *) packet;
    int i;
	
    for (i = 0; i < packet->header.length; i++) 
	{ //XOR checksum value with packet data 
        checksum ^= *p;
        p++;
    }
    return (int) checksum;
}

void sendMessage(int seq, char *data, int n) 
{  
    // initializes packet with data
    int i;
    for (i = 0; i < n; i++) 
	{
        p->data[i] = data[i];
    }

    // Sets header of packet
    p->header.sequence_ack = seq;
    p->header.length = n;
    int checksum = checks(p);
    p->header.checksum = checksum;
    
    fd_set readfds;
    fcntl(sock, F_SETFL, O_NONBLOCK);
  
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    
    // This will send data to server
    sendto(sock, p, sizeof(packet), 0, (struct sockaddr *)&serverAddr, addr_size);
    if (n == 0)
      times_sent_zero++;
    else 
      times_sent_zero = 0;
    
    // setting the timer
    tv.tv_sec = 2;
    tv.tv_usec = 2;
    
    // this will call select
    rv = select(sock+1, &readfds, NULL, NULL, &tv);
    
    // if client keeps sending to a non-responsive server
    if (times_sent_zero == 3) 
	{
	close(sock);
	return;
    }
    
    if (rv == 0) 
	{ // there is a timeout
        printf("TIMEOUT!\n\n");
		// resend data
        sendMessage(seq_num, data, nBytes); 
    }
}

void receiveMessage() 
{
    int mystate = 0;
    int checksum;

    while (mystate == 0) 
	{
        //receives ack
        nBytes = recvfrom(sock, r, sizeof(received), 0, NULL, NULL);
		
        int Rchecksum = r->header.checksum;
		
        // random condition to check correct checksum or not to test errors
        if (rand() % 100 > 2)
            checksum = checks(r);
        else
            checksum = 0;

        if (checksum == Rchecksum && r->header.sequence_ack == seq_num) 
		{
            mystate = 1;
			// flip
			if (seq_num == 0)
				seq_num = 1;
			else
				seq_num = 0;
        }
		
		else 
		{ // resend
            sendMessage(p->header.sequence_ack, p->data, sizeof(p->data));
        }
    }
}
