#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <iostream>
#include <string>
#include <math.h>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         10000
#define HOST_NAME_SIZE      255
#define NSTD 3

using namespace std;

int parseArgs(char* args[], int &argCount, char host[], int &port, 
		char file[], int &socketCount, bool &debug)
{
	extern char *optarg;
	int c = 1;
	int err = 0;

	while((c = getopt(argCount, args, "c:d")) != -1)
	{
		switch(c)
		{
			case 'c':
				socketCount = atoi(optarg);
				break;
			case 'd':
				debug = true;
				break;
			case '?':
				err = 1;
				break;
		}
	}

	strcpy(host, args[optind]);
	port = atoi(args[optind + 1]);
	strcpy(file, args[optind + 2]);
	return err;
}

float getMean(float nums[], int numsSize)
{
	float total = 0;
	for (int i = 0; i < numsSize; ++i)
	{
		total += nums[i];
	}
	return total / numsSize;
}

float getStdDev(float nums[], int numsSize, float mean)
{
	float total = 0;
	for (int i = 0; i < numsSize; ++i)
	{
		total += pow((nums[i] - mean), 2);
	}
	float sdMean = total / numsSize;
	return sqrt(sdMean);
}

int  main(int argc, char* argv[])
{
	char strHostName[HOST_NAME_SIZE];
	char strFile[HOST_NAME_SIZE];
	int port = -1;
	int socketCount = 1;
	bool debug = false;

	if (argc < 4 || parseArgs(argv, argc, strHostName, port, strFile, socketCount, debug) == 1)
	{
		printf("\nUsage: client host-name host-port");
		printf("\n    host-name: host address");
		printf("\n    host-port: port as integer");
		printf("\n    file: file to retrieve");
		printf("\n    optional:");
		printf("\n        -d: for debug info");
		printf("\n        -c NUM: download NUM amount of times\n\n");
		return 1;
	} else if (debug)
	{
		printf("\nDEBUG MODE ON\n");
		printf("\nConnecting to %s on port %d to retrieve %s %d times\n", 
			strHostName, port, strFile, socketCount);
	}

    int hSocket[socketCount];
    struct timeval oldtime[socketCount + NSTD];
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];

    /* make a socket */
    if (debug)
    	printf("Making a socket\n");

	for(int i = 0; i < socketCount; i++) {
	    
	    hSocket[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(hSocket[i] == SOCKET_ERROR)
		{
			printf("\nCould not make a socket\n");
			return 2;
		}
	}

    /* get IP address from name */
    pHostInfo = gethostbyname(strHostName);

    if (pHostInfo == 0)
    {
    	printf("\nCould not get host name\n");
    	return 3;
    }

    /* copy address into long */
    memcpy(&nHostAddress, pHostInfo->h_addr, pHostInfo->h_length);

    /* fill address struct */
    Address.sin_addr.s_addr = nHostAddress;
    Address.sin_port = htons(port);
    Address.sin_family = AF_INET;
	
	// Send the requests and set up the epoll data
	int epollFD = epoll_create(1);
	for(int i = 0; i < socketCount; i++) {

		/* connect to host */
		if(connect(hSocket[i], (struct sockaddr*)&Address, sizeof(Address)) 
		   == SOCKET_ERROR)
		{
			printf("\nCould not connect to host\n");
			return 0;
		}

		char request[HOST_NAME_SIZE + 20];
		sprintf(request, "GET %s HTTP/1.0\r\n\r\n", strFile);
		write(hSocket[i], request, strlen(request));

		// Keep track of the time when we sent the request
		gettimeofday(&oldtime[hSocket[i]], NULL);

		// Tell epoll that we want to know when this socket has data
		struct epoll_event event;
		event.data.fd = hSocket[i];
		event.events = EPOLLIN;
		int ret = epoll_ctl(epollFD, EPOLL_CTL_ADD, hSocket[i], &event);
		if(ret)
			perror ("epoll_ctl");
	}

	float times[socketCount];
	for(int i = 0; i < socketCount; i++) {
		struct epoll_event event;
		int rval = epoll_wait(epollFD, &event, 1, -1);

		if(rval < 0)
			perror("epoll_wait");

		rval = read(event.data.fd, pBuffer, BUFFER_SIZE);
		
		// Get the current time and subtract the starting time for this request.
		struct timeval newtime;
		gettimeofday(&newtime, NULL);
		double usec = (newtime.tv_sec - oldtime[event.data.fd].tv_sec) * (double)1000000 + 
				(newtime.tv_usec - oldtime[event.data.fd].tv_usec);
		usec /= 1000000;
		times[i] = usec;
		
		if (debug)
			printf("Got %d from %d in %f seconds\n", rval, event.data.fd, usec);

		// Take this one out of epoll
		epoll_ctl(epollFD, EPOLL_CTL_DEL, event.data.fd, &event);
	}

	// Calculate Mean
	float mean = getMean(times, socketCount);
	// Calculate Standard Deviation
	float stdDev = getStdDev(times, socketCount, mean);
	// Print Results
	printf("\nAverage             =  %f seconds", mean);
	printf("\nStandard Deviation  =  %f seconds\n", stdDev);

	// Now close the sockets
	if (debug) 
		printf("\nClosing sockets\n");
	for(int i = 0; i < socketCount; i++) {
		
		/* close socket */                       
		if(close(hSocket[i]) == SOCKET_ERROR)
		{
			printf("\nCould not close socket %d\n", i);
			return 0;
		}
	}

	printf("\n");
}
