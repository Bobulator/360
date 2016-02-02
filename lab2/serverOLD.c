#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <dirent.h>

#include "serverUtils.cpp"

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         60000
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
void serveRequest(const int &hSocket, const string &sDirectory, const int &nHostPort);
void closeSocket(const int &socket);

int main(int argc, char* argv[])
{
    int hSocket, hServerSocket;  /* handle to socket */
    //struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize = sizeof(struct sockaddr_in);
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

        serveRequest(hSocket, sDirectory, nHostPort);

        // Wait for client to receive data in transit before closing socket
        setLinger(hSocket);
        closeSocket(hSocket);
    }
}


void serveRequest(const int &hSocket, const string &sDirectory, const int &nHostPort)
{
    stringstream ss;
    string sHeaders;
    string sFileContents = "";
    char *fBuff = NULL;
    char directoryName[255];

    printf("\nRetrieving headers.....");
    vector<char *> headers;
    GetHeaderLines(headers, hSocket, false);
    printf("DONE\n");

    if (headers.size() == 0)
    {
        printf("\nCould not parse request\n");
        return;
    }

    printf("\nGot from browser: \n");
    printVector(headers);
    printf("\n");

    printf("Retrieving requested file/directory name.....");
    sprintf(directoryName, "%s", (sDirectory + getRequestedFileName(headers)).c_str());
    printf("DONE\n");

    printf("\nRequested file/directory: %s\n\n", directoryName);

    struct stat filestat;

    if (stat(directoryName, &filestat))
    {
        sHeaders = "HTTP/1.1 404 error\r\n\r\n";
        sFileContents = "404 File Not Found";
    }

    else if (S_ISREG(filestat.st_mode))
    {
        sFileContents = fileToString(directoryName, fBuff, filestat);

        ss << "HTTP/1.1 OK\r\nContent-Type: " 
           << getExt(directoryName)
           << "\r\nContent-Length: "
           << (int) sFileContents.size()
           << "\r\n\r\n";
        sHeaders = ss.str();
    }

    else if (S_ISDIR(filestat.st_mode))
    {
        struct stat buffer;
        string name = directoryName;
        name = name + "index.html";
        if (stat (name.c_str(), &buffer) == 0)
        {
            printf ("index.html found in directory\n");
            sFileContents = fileToString(name.c_str(), fBuff, buffer);

            ss << "HTTP/1.1 OK\r\n"
               << "Content-Type: text/html\r\n"
               << "Content-Length: "
               << (int) sFileContents.size()
               << "\r\n\r\n";
            sHeaders = ss.str();
        }
        else
        {
            printf("index.html not find, sending directory contents\n");

            stringstream ts;
            ts << "<html>\n"
            << "<body>\n"
            << "<ol>\n";
            
            DIR *dirp;
            struct dirent *dp;

            dirp = opendir(directoryName);
            while ((dp = readdir(dirp)) != NULL)
            {
                ts << "<li><a href=" << dp->d_name 
                   << ">/" << dp->d_name
                   << "</a>\n";
            }
            (void)closedir(dirp);
            ts << "</ol>"
               << "</body>\n"
               << "</html>";

            sFileContents = ts.str();

            ss << "HTTP/1.1 OK\r\n"
               << "Content-Type: text/html\r\n"
               << "Content-Length: "
               << sFileContents.size()
               << "\r\n\r\n";
            sHeaders = ss.str();
        }
    }

    //printf("\n%d\n%s\n", (int) sHeaders.size(), sHeaders.c_str());
    //printf("\n%d%s\n", (int) sFileContents.size(), sFileContents.c_str());
    //printf("\n\nFILE SIZE: %d\n\n", (int) sFileContents.size());
    string response = sHeaders + sFileContents;
    int wroteAmt = write(hSocket, response.c_str(), response.size());
    printf("Wrote %d to socket", wroteAmt);
    
    if (fBuff != NULL)
        free(fBuff);
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
    lin.l_onoff = 1;
    lin.l_linger = 10;
    setsockopt(hSocket, SOL_SOCKET, SO_LINGER, &lin, sizeof(lin));
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
