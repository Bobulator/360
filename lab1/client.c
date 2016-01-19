#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define SOCKET_ERROR        -1
#define RESPONSE_SIZE         1024
#define HOST_NAME_SIZE      255
#define MAXGET 1024

bool isNumber(char number[])
{
    int i;
    for (i = 0; number[i] != 0; ++i)
    {
        if (!isdigit(number[i]))
        {
            return false;
        }
    }
    return true;
}

int  main(int argc, char* argv[])
{
    int hSocket;                 /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[RESPONSE_SIZE];
    unsigned nReadAmount;
    char strHostName[HOST_NAME_SIZE];
    char strFilePath[HOST_NAME_SIZE];
    int nHostPort;
    extern char *optarg;
    int i;
    int nTimesToDownload = -1, err = 0;
    int nNonOptArgs = -1;
    bool debug = false;

    if (argc < 4 || 7 < argc)
        err = 1;
    else
    {
        for (i = 1; i < argc; ++i) 
        {
            if (strcmp(argv[i], "-c") == 0)
            {
                if (i + 1 < argc && isNumber(argv[i + 1]))
                {
                    i++;
                    nTimesToDownload = atoi(argv[i]);
                }
                else
                {
                    err = 2;
                    break;
                }
            } 
            else if (strcmp(argv[i], "-d") == 0)
                debug = true;
            else
            {
                nNonOptArgs++;
                if (nNonOptArgs == 0)
                    strncpy(strHostName, argv[i], HOST_NAME_SIZE);
                else if (nNonOptArgs == 1 && isNumber(argv[i]))
                    nHostPort = atoi(argv[i]);
                else if (nNonOptArgs == 2)
                    strncpy(strFilePath, argv[i], HOST_NAME_SIZE);
                else 
                {
                    err = 3;
                    break;
                }
            }
        }
    }
    

    if (err > 0 || nNonOptArgs < 2)
    {
        printf("\nError %d\nUsage: client host-name host-port path -c or -d\n  host-port: must be an integer\n", err);
        return 1;
    }

    if (debug == 1)
    {
        printf("\nHost-name: %s\nHost-port: %d\nPath: %s\n", strHostName, nHostPort, strFilePath);
        printf("\nDEBUG: %d\nTIMES_TO_DOWNLOAD: %d\n", debug, nTimesToDownload);
    }
    
    printf("\nMaking a socket");
    /* make a socket */
    hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if(hSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        return 0;
    }

    /* get IP address from name */
    pHostInfo=gethostbyname(strHostName);
    
    // If pHostInfo is not null
    if (pHostInfo)
    {
        /* copy address into long */
        memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);
    }
    else
    {
        printf("\nERROR: could not get address from host name\n");
        return 2;
    }
    

    /* fill address struct */
    Address.sin_addr.s_addr=nHostAddress;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    printf("\nConnecting to %s (%X) on port %d",strHostName,(unsigned int)nHostAddress,nHostPort);

    /* connect to host */
    if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address)) 
       == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }

    /* read from socket into buffer
    ** number returned by read() and write() is the number of bytes
    ** read or written, with -1 being that an error occured */

    // Create HTTP Message
    char *message = (char *)malloc(MAXGET);
    sprintf(message, "\nGET %s HTTP/1.1\r\nHost:%s:%d\r\n\r\n",strFilePath,strHostName,nHostPort);

    // Send HTTP on the socket
    if (debug == 1)
        printf("\n\nRequest: %s\n", message);

    // Read Response back from socket
    int nSuccessfulDownloads = 0;
    if (nTimesToDownload > -1)
    {
        for (i = 1; i <= nTimesToDownload; ++i) {
            int temp = write(hSocket,message,strlen(message));
            nReadAmount=read(hSocket,pBuffer,RESPONSE_SIZE);
            if (nReadAmount >= 0)
                nSuccessfulDownloads++;
        }
        printf("\nSuccessful downloads: %d ", nSuccessfulDownloads);
    }
    else
    {
        write(hSocket,message,strlen(message));
        nReadAmount=read(hSocket,pBuffer,RESPONSE_SIZE);
        printf("\nRead: %d\n\nResponse: ", nReadAmount);
        fwrite(pBuffer, nReadAmount, 1, stdout);
    }

    printf("\nClosing socket\n");
    /* close socket */                       
    if(close(hSocket) == SOCKET_ERROR)
    {
        printf("\nCould not close socket\n");
    }

    free(message);

    return 0;
}
