#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "cs360Utils.h"

using namespace std;

#define SOCKET_ERROR        -1
#define QUEUE_SIZE          5
#define BAD_REQUEST         "400 Bad Request"
#define NOT_FOUND           "404 Not Found"


/******************************************************************************
******************************* HELPER FUNCTIONS ******************************
******************************************************************************/
bool checkArgs(char* argv[], int argc)
{
    if (argc != 3)
    {
        printf("\nUsage: server host-port host-dir\n");
        return false;
    }
    else
        return true;
}

string generateErrorHeader(string error)
{
    stringstream ss;
    ss << "HTTP/1.1 " << error << "\r\n\r\n";
    return ss.str();
}

string generateResponseHeaders(string cType, int cSize)
{
    stringstream ss;
    ss << "HTTP/1.1 OK\r\n"
       << "Content-Type: " << cType << "\r\n"
       << "Content-Length: " << cSize << "\r\n\r\n";
       return ss.str();
}

/******************************************************************************
**************************** MAIN SERVER FUNCTIONS ****************************
******************************************************************************/

/** This function processes the request in the given socket using the given
  * directory as a root path.
  **/
void serve(const int hSocket, const string &sDirectory)
{
    string sHeaders;
    string sBody;
    string sRequest;
    string sPath;

    cout << "\nParsing Headers......\n";
    vector<char *> headers;
    GetHeaderLines(headers, hSocket, false);
    cout << "DONE\n";

    struct stat filestat;
    if (headers.size() < 1 || 
        (!strncmp(headers[0], "HTTP_GET", 8) && 
           !strncmp(headers[0], "GET", 3)))
    {
        sHeaders = generateErrorHeader(BAD_REQUEST);
        sBody = BAD_REQUEST;
    }
    else
    {
        cout << "\nGot from browser:\n";
        PrintVector(headers);
        cout << "\n";

        sRequest = GetRequestedFileName(headers);
        cout << "Requested: " << sRequest << "\n";

        sPath = sDirectory + sRequest;

        if (stat(sPath.c_str(), &filestat))
        {
            sHeaders = generateErrorHeader(NOT_FOUND);
            sBody = NOT_FOUND;
        }
        else if (S_ISREG(filestat.st_mode))
        {
            sBody = FileToString(sPath.c_str(), filestat);
            sHeaders = generateResponseHeaders(GetExt(sPath.c_str()), sBody.size());
        }
        else if (S_ISDIR(filestat.st_mode))
        {
            struct stat indexStat;
            string indexPath = sPath + "/index.html";

            if (stat(indexPath.c_str(), &indexStat) == 0)
                sBody = FileToString(indexPath.c_str(), indexStat);
            else
                sBody = DirectoryToHtml(sPath.c_str());

            sHeaders = generateResponseHeaders("text/html", sBody.size());
        }
    }
    cout << sHeaders << "\n";
    string sResponse = sHeaders + sBody;
    write(hSocket, sResponse.c_str(), sResponse.size());

    cout << "\nClosing the socket";
    if(close(hSocket) == SOCKET_ERROR)
    {
        cout << "\nCoult not clost the socket\n";
        exit(0);
    }
}

int main(int argc, char* argv[])
{
    int hSocket;
    int hServerSocket;
    struct sockaddr_in Address;
    int nAddressSize = sizeof(struct sockaddr_in);
    int nHostPort;
    string sDirectory;

    // Verify arguments
    if (checkArgs(argv, argc))
    {
        nHostPort = atoi(argv[1]);
        sDirectory = argv[2];
    }
    else
        return 0;


    printf("\nStarting server");

    printf("\nMaking socket");
    hServerSocket = socket(AF_INET,SOCK_STREAM,0);

    if (hServerSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        return 0;
    }

    // Make socket reusable
    int optval = 1;
    int setSockResult = setsockopt(hServerSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (setSockResult == SOCKET_ERROR)
        printf("\nFailed to make socket reusable\n");

    // Fill in address struct
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    printf("\nBinding to port %d",nHostPort);
    if (bind(hServerSocket, (struct sockaddr*) &Address, sizeof(Address)) 
        == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }

    getsockname(hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);
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
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
        return 0;
    }

    

    for(;;)
    {
        cout << "\nWaiting for a connection\n";
        hSocket = accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);

        printf("\nGot a connection from %X (%d)\n", Address.sin_addr.s_addr, ntohs(Address.sin_port));
        serve(hSocket, sDirectory);
    }
}