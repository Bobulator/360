#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         10000
#define MESSAGE             "This is the message I'm sending back and forth"
#define QUEUE_SIZE          5

int main(int argc, char* argv[])
{
    int hSocket,hServerSocket;  /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize=sizeof(struct sockaddr_in);
    char pBuffer[BUFFER_SIZE];
    int nHostPort;

    if(argc < 3)
      {
        printf("\nUsage: server host-port dir\n");
        return 0;
      }
    else
      {
        nHostPort=atoi(argv[1]);
      }

    printf("\nStarting server");

    printf("\nMaking socket");
    /* make a socket */
    hServerSocket=socket(AF_INET,SOCK_STREAM,0);

    if(hServerSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        return 0;
    }

    /* fill address struct */
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    printf("\nBinding to port %d",nHostPort);

    /* bind to a port */
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) 
                        == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }
 /*  get port number */
    getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);
    printf("opened socket as fd (%d) on port (%d) for stream i/o\n",hServerSocket, ntohs(Address.sin_port) );

        printf("Server\n\
              sin_family        = %d\n\
              sin_addr.s_addr   = %d\n\
              sin_port          = %d\n"
              , Address.sin_family
              , Address.sin_addr.s_addr
              , ntohs(Address.sin_port)
            );


    printf("\nMaking a listen queue of %d elements",QUEUE_SIZE);
    /* establish listen queue */
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
        return 0;
    }
int optval = 1;
setsockopt (hServerSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    for(;;)
    {
        printf("\nWaiting for a connection\n");
        /* get the connected socket */
        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);

        printf("\nGot a connection from %X (%d)\n",
              Address.sin_addr.s_addr,
              ntohs(Address.sin_port));
        memset(pBuffer,0,sizeof(pBuffer));
        int rval = read(hSocket,pBuffer,BUFFER_SIZE);
	printf("Got from browser %d\n%s\n",rval, pBuffer);
#define MAXPATH 1000
	char path[MAXPATH];
	rval = sscanf(pBuffer,"GET %s HTTP/1.1",path);
	printf("Got rval %d, path %s\n",rval,path);
	char fullpath[MAXPATH];
	sprintf(fullpath,"%s%s",argv[2], path);
	printf("fullpath %s\n",fullpath);
	

        memset(pBuffer,0,sizeof(pBuffer));
	sprintf(pBuffer,"HTTP/1.1 200 OK\r\n\
Content-Type: image/jpg\r\n\
Content-Length: 51793\
\r\n\r\n");
	write(hSocket,pBuffer, strlen(pBuffer));
	FILE *fp = fopen("test4.jpg","r");
	char *buffer = (char *)malloc(51793+1);
	fread(buffer, 51793, 1,fp);
	write(hSocket,buffer,51793);
	free(buffer);
	fclose(fp);
#ifdef notdef
linger lin;
unsigned int y=sizeof(lin);
lin.l_onoff=1;
lin.l_linger=10;
setsockopt(hSocket,SOL_SOCKET, SO_LINGER,&lin,sizeof(lin));	
shutdown(hSocket, SHUT_RDWR);
#endif
    printf("\nClosing the socket");
        /* close socket */
        if(close(hSocket) == SOCKET_ERROR)
        {
         printf("\nCould not close socket\n");
         return 0;
        }
    }
}
