#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "serverUtils.cpp"

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         1024
#define QUEUE_SIZE          5

using namespace std;

// functions
void printResults(const struct sockaddr_in &Address, const string &sDirectory);
void makeSocket(int &hServerSocket);
void fillAddressStruct(struct sockaddr_in &Address, const int &nHostPort);
void bindToPort(const int &nHostPort, const int &hServerSocket, const struct sockaddr_in &Address);
void openSocket(const int &hServerSocket, const struct sockaddr_in &Address, const int &nAddressSize);
void startQueue(const int &hServerSocket);
void setReusableSocket(const int &hServerSocket);
void listenForConnection();
void setLinger(const int &hSocket);
int acceptConnection(const int &hServerSocket, const struct sockaddr_in &Address, const int &nAddressSize);
void serveRequest(const int &hSocket, const string &sDirectory, char *pBuffer, const int &pBufferSize);
void closeSocket(const int &socket);

int main(int argc, char* argv[])
{
    int hSocket, hServerSocket;  /* handle to socket */
    //struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize = sizeof(struct sockaddr_in);
    char pBuffer[BUFFER_SIZE];
    int nHostPort;
    string sDirectory;

    if(argc != 3)
    {
        printf("\nUsage: server host-port directory\n");
        printf("    host-port: port to bind to socket\n");
        printf("    directory: directory to fetch GET requests from\n\n");
        return 0;
    }
    else
    {
        nHostPort = atoi(argv[1]);
        sDirectory = argv[2];
    }

    printf("\nStarting server");
    
    /* make a socket */
    makeSocket(hServerSocket);

    // Allow socket to be reusable immediately
    setReusableSocket(hServerSocket);

    /* fill address struct */
    fillAddressStruct(Address, nHostPort);

    /* bind to a port */
    bindToPort(nHostPort, hServerSocket, Address);

    /*  get port number */
    openSocket(hServerSocket, Address, nAddressSize);

    // Print results
    printResults(Address, sDirectory);
    
    /* establish listen queue */
    startQueue(hServerSocket);

    

    for(;;)
    {
        printf("\nWaiting for a connection\n");

        /* get the connected socket */
        hSocket = acceptConnection(hServerSocket, Address, nAddressSize);

        serveRequest(hSocket, sDirectory, pBuffer, (int) sizeof(pBuffer));

        closeSocket(hSocket);
    }
}


void serveRequest(const int &hSocket, const string &sDirectory, char *pBuffer, const int &pBufferSize)
{
    printf("\nRetrieving headers...");
    vector<char *> headers;
    GetHeaderLines(headers, hSocket, false);
    printf("DONE\n");

    printf("\nGot from browser: \n");
    printVector(headers);
    printf("\n");

    printf("Retrieving requested file/directory name...");
    string fileOrDirName = getRequestedFileName(headers);
    printf("DONE\n");

    printf("\nRequested file/directory: %s\n\n", fileOrDirName.c_str());
    memset(pBuffer, 0, pBufferSize);
    int readResult = readFileOrDirectory((sDirectory + fileOrDirName).c_str(), pBuffer, pBufferSize);

    char responseBuffer[2048];
    if (readResult == -1)
    {
        sprintf(responseBuffer, "HTTP/1.1 404\r\n\r\n404 File Not Found");
    }

    else if (readResult == 1)
    {
        sprintf(responseBuffer, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n\r\n%s\n", pBuffer);
    }

    else if (readResult == 2)
    {

    }

    write(hSocket, responseBuffer, strlen(responseBuffer)); 

    // Wait for client to receive data in transit before closing socket
    setLinger(hSocket);
}


void printResults(const struct sockaddr_in &Address, const string &sDirectory)
{
    printf("Server\n\
      sin_family        = %d\n\
      sin_addr.s_addr   = %d\n\
      sin_port          = %d\n\
      working directory = %s\n"
      , Address.sin_family, Address.sin_addr.s_addr
      , ntohs(Address.sin_port), sDirectory.c_str());
    printf("\nMaking a listen queue of %d elements\n",QUEUE_SIZE);
}


void makeSocket(int &hServerSocket)
{
    printf("\nMaking socket");
    hServerSocket = socket(AF_INET,SOCK_STREAM, 0);

    if(hServerSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        exit(0);
    }
}


void fillAddressStruct(struct sockaddr_in &Address, const int &nHostPort)
{
    Address.sin_addr.s_addr = INADDR_ANY;
    Address.sin_port = htons(nHostPort);
    Address.sin_family = AF_INET;
}


void bindToPort(const int &nHostPort, const int &hServerSocket, const struct sockaddr_in &Address)
{
    printf("\nBinding to port %d", nHostPort);
    if(bind(hServerSocket, (struct sockaddr*) &Address, sizeof(Address)) 
        == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        exit(0);
    }
}


void openSocket(const int &hServerSocket, const struct sockaddr_in &Address, const int &nAddressSize)
{
    getsockname( hServerSocket, (struct sockaddr *) &Address, (socklen_t *) &nAddressSize);
    printf("opened socket as fd (%d) on port (%d) for stream i/o\n",
        hServerSocket, ntohs(Address.sin_port));
}


void startQueue(const int &hServerSocket) 
{
    if(listen(hServerSocket, QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
        exit(0);
    }
}


void setReusableSocket(const int &hServerSocket)
{
    int optval = 1;
    int setSockResult = setsockopt(hServerSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (setSockResult == -1)
    {
        printf("\nFailed to make socket reusable\n");
    }
}


void setLinger(const int &hSocket)
{
    linger lin;
    unsigned int lin_size = sizeof(lin);
    lin.l_onoff = 1;
    lin.l_linger = 10;
    setsockopt(hSocket, SOL_SOCKET, SO_LINGER, &lin, lin_size);
    shutdown(hSocket, SHUT_RDWR);
}


int acceptConnection(const int &hServerSocket, const struct sockaddr_in &Address, const int &nAddressSize)
{
    int result = accept(hServerSocket, (struct sockaddr*) &Address, (socklen_t *) &nAddressSize);
    if (result == -1)
    {
        printf("\nFailed to accept connection\n");
        exit(0);
    }

    printf("\nGot a connection from %X (%d)\n",
        Address.sin_addr.s_addr,
        ntohs(Address.sin_port));

    return result;
}


void closeSocket(const int &socket)
{
    printf("\nClosing the socket");
    /* close socket */
    if(close(socket) == SOCKET_ERROR)
    {
        printf("\nCould not close socket\n");
        exit(0);
    }
}
