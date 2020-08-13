//	Christian Quintero 
//	COEN146L
//	udpclient.c
//
//	This file uses UDP connection to send a file name for output to a server
//	and then reads the data from an input file to be sent to the server.

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#define <BUFF_SIZE> 10

typedef struct {
  int sequence_ack;
  int length;
} HEADER;

typedef struct {
  HEADER header;
  char data[BUFF_SIZE];
} PACKET;

PACKET packet;
PACKET *p = &packet;
PACKET received;
PACKET *r = &received;

int seq_num = 0;
int nBytes;
int sock;
struct sockaddr_in serverAddr;
socklen_t addr_size;

void sendMessage(int seq, char *data, int n);
void receiveMessage();

/***********
 *  main
 ***********/
int main (int argc, char *argv[])
{
  char buffer[BUFF_SIZE];

  // Configure address
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons (atoi(argv[2]));
  inet_pton(AF_INET, argv[1], &serverAddr.sin_addr.s_addr);
  memset(serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));  
  addr_size = sizeof serverAddr;

  // Create UDP socket 
  sock = socket(PF_INET, SOCK_DGRAM, 0);
  
  // Sends the filename for output to server
  sendMessage(seq_num, argv[4], strlen(argv[4])+1);
  // Checks to see if sent successfully
  receiveMessage();

  FILE *fp_in = fopen(argv[3], "r");
 
  // receives data from input then sends data to server
  while(nBytes = fread(buffer, 1, 10, fp_in)) 
  {
    sendMessage(seq_num, buffer, nBytes);
	
	// checks if data was sent correctly
    receiveMessage();
  }
  
  fclose(fp_in);
  
  // Sends a final empty message
  sendMessage(seq_num, "", 0);
  
  close(sock);
  
  return 0;
}

void sendMessage(int seq, char *data, int n) 
{
  // Sets packet data
  int i;
  for (i = 0; i < n; i++) 
  {
    p->data[i] = data[i];
  }

  p->header.sequence_ack = seq;
  p->header.length = n;

  // Send packet data to the server
  sendto(sock, p, sizeof(packet), 0, (struct sockaddr *)&serverAddr, addr_size);
}

void receiveMessage() 
{
  int mystate = 0;
  // checks to resend package or not
  while (mystate == 0) 
  {
    // Receive ack from server
    nBytes = recvfrom(sock, r, sizeof(received), 0, NULL, NULL);

    if (r->header.sequence_ack == seq_num) 
    {
      mystate = 1;
    } 
    else 
    { 
      sendMessage(p->header.sequence_ack, p->data, sizeof(p->data));
    }
  }
  
  // Flip seq_num
  if (seq_num == 0)
    seq_num = 1;
  else
    seq_num = 0;
}
