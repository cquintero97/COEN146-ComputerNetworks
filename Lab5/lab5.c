// Christian Quintero
// COEN 146L
// lab5.c
//
// This file simulates routing by running the link-state algorithm and uses
// a costs and machines files to hold distance costs between nodes N and 
// machines info.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h>
#include <arpa/inet.h>
#include <unistd.h>

struct machine 
{
    char name[50];
    char ip[50];
    int port;
};

int routerID, numNodes;
char costsFN[50];
char hostsFN[50];
int costs[10][10];

struct machine hosts[10];

void *link_state();
void *receive_info();
void send_info(int *dataSending, int size_info);

pthread_mutex_t lock;
pthread_t link_state_thread;
pthread_t receive_info_thread;

int sock;
struct sockaddr_in serverAddr, target_addr;
socklen_t addr_size;

// use this function to return min btwn two ints
int min(int a, int b) 
{
    if (a > b)
        return b;
    return a;
}

int main(int argc, char *argv[]) 

{
    int dataSending[3];
	
    routerID = atoi(argv[1]);
    numNodes = atoi(argv[2]);
    strcpy(costsFN, argv[3]);
    strcpy(hostsFN, argv[4]);

    // initialize mutex and rand()
    pthread_mutex_init(&lock, NULL);
    srand(time(NULL));
    
    // parses costs data file and store in costs array
    FILE *f_costs = fopen(costsFN, "r");
    int i;
	int j;
    for (i = 0; i < numNodes; i++) 
	{
        for (j = 0; j < numNodes; j++) 
		{
            fscanf(f_costs, "%d", &costs[i][j]);
        }
    }
    fclose(f_costs);
    
    // parses hosts data file and store in hosts array
    FILE *f_hosts = fopen(hostsFN, "r");
    for (i = 0; i < numNodes; i++) 
	{
        fscanf(f_hosts, "%s %s %d", hosts[i].name, hosts[i].ip, &hosts[i].port);
    }
    fclose(f_hosts);
    
    // set up connection
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons((short)hosts[routerID].port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset((char*)serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    addr_size = sizeof serverAddr;
    
    // create UDP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
		printf("socket error\n");
		return 1;
    }
	
    // bind
    int bindsaved;
    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0)
    {
		printf("bind error\n");
		return 1;
    }

    // creates my link_state thread and receive_info_thread
    pthread_create(&link_state_thread, NULL, link_state, NULL);
    pthread_create(&receive_info_thread, NULL, receive_info, NULL);

   
    int changes = 0;
    int msec = 0;
    time_t before;
    
    int neighborID;
    int newCost;
    do { // will delay for 30 seconds after 3rd change has been made
		int msec = 0;	
        time_t before = clock();
        
        // request user input
        printf("Ready for user input in format: [neighborID] [newCost]\n");
		scanf("%d %d", &neighborID, &newCost); // receive id and new_cost from user input
        printf("\n");
		
        // update neighbor costs table
        costs[routerID][neighborID]=newCost;
        costs[neighborID][routerID]=newCost;
        changes++;
        
        // update dataSending 
        dataSending[0] = htonl(routerID);
        dataSending[1] = htonl(neighborID);
        dataSending[2] = htonl(newCost);
        
        // send updates to rest of hosts
        send_info(dataSending, sizeof(dataSending));
        
        // sleep for 10 seconds
        sleep(10);
        
        if (changes == 3) 
		{	 
            before = clock();
            changes++;
        } 
		else if (changes > 3) 
		{
            clock_t difference = clock() - before;
            msec = difference*1000/CLOCKS_PER_SEC;
        }
    } while (msec < 30000); 
	
    return 0;
}

void send_info(int *dataSending, int size_info) 
{
    int i;
	// send data to all other nodes
    for (i = 0; i < numNodes; i++) 
	{ 
        if (i != routerID) 
		{
				target_addr.sin_family = AF_INET;
        	    target_addr.sin_port = htons((short)hosts[i].port);
        	    inet_pton(AF_INET, hosts[i].ip, &target_addr.sin_addr.s_addr);
        	    sendto(sock, dataSending, size_info, 0, (struct sockaddr *)&target_addr, sizeof(target_addr));
        }
    }
}

void *link_state() 
{
    int dist[10];
    int visited[10];
    
    while (1) 
	{
		pthread_mutex_lock(&lock);
		// Djkstra's algorithm
		int i;
		for (i = 0; i < numNodes; i++) 
		{
			dist[i] = costs[routerID][i]; 
            visited[i] = 0; // if visited then 1, else 0
		}
		visited[routerID] = 1;
		int min_index;
		int counter=0;
		printf("routerID: %d\n", routerID);
		while(counter<3)
		{
			int min_distance = INT_MAX;
        	for (i = 0; i < numNodes; i++) 
			{
           	 	printf("minindexbefore: %d, Dist[i]: %d ,   minddist:   %d\n", min_index, dist[i], min_distance);
	    		if (visited[i] == 0 && dist[i] < min_distance) 
	    		{
				printf("in if\n");
      		        	min_index = i;
               			min_distance = dist[min_index];
				}
			}
        	printf("minindex: %d\n", min_index);
        	visited[min_index] = 1;
        	int j;
        	for (j = 0; j < numNodes; j++) 
			{
                	if (visited[j] == 0) 
					{
						printf("%d  %d  %d\n", dist[j], dist[min_index], costs[min_index][j]); 
                 	    dist[j] = min(dist[j], dist[min_index] + costs[min_index][j]);
                	}	
            }
        	counter++;
	}
	pthread_mutex_unlock(&lock);
        
        printf("Least cost array: \n[");
        for (i = 0; i < numNodes; i++)
		{
            printf(" %d ", dist[i]);
        }
		printf("]\n\n");
        
        // sets a delay for 10-20 secs
        int delay = 10+rand()%10;
        sleep(delay);
    }
}

void *receive_info() 
{
    int nBytes, host_id, next_host_id, newCost;
    int data_received[3];
    
    while (1) 
	{
        // receive data
        nBytes = recvfrom(sock, data_received, 1024, 0, NULL, NULL);
        printf("received\n"); 
        // if there was data received
        if (sizeof(data_received) != 0) {
            host_id = ntohl(data_received[0]);
            next_host_id = ntohl(data_received[1]);
            newCost = ntohl(data_received[2]);
        
            // update
            pthread_mutex_lock(&lock);
            costs[host_id][next_host_id] = newCost;
            costs[next_host_id][host_id] = newCost;
            pthread_mutex_lock(&lock);
        }
    }
}
