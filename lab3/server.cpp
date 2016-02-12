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
#include <semaphore.h>
#include <queue>
#include <pthread.h>
#include <signal.h>

#include "cs360Utils.h"

using namespace std;

#define SOCKET_ERROR        -1
#define QUEUE_SIZE          20
#define BAD_REQUEST         "400 Bad Request"
#define NOT_FOUND           "404 Not Found"

sem_t empty, full, mutex;

struct thread_params
{
    long threadID;
    string dir;
};


/******************************************************************************
******************************** QUEUE WRAPPER ********************************
******************************************************************************/
class MyQueue
{
    std::queue <int> stlqueue;
public:
    void push(int sock)
    {
        sem_wait(&empty);
        sem_wait(&mutex);
        stlqueue.push(sock);
        sem_post(&mutex);
        sem_post(&full);
    }
    int pop()
    {
        sem_wait(&full);
        sem_wait(&mutex);
        int rval = stlqueue.front();
        stlqueue.pop();
        sem_post(&mutex);
        sem_post(&empty);
        return rval;
    }
} sockqueue;

/******************************************************************************
******************************* HELPER FUNCTIONS ******************************
******************************************************************************/
bool checkArgs(char* argv[], int argc)
{
    if (argc != 4)
    {
        printf("\nUsage: server host-port threads host-dir\n");
        printf("    host-port: integer\n");
        printf("    threads: integer\n");
        printf("    host-dir: directory\n");
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

void initSems()
{
    sem_init(&mutex, PTHREAD_PROCESS_PRIVATE, 1);
    sem_init(&full, PTHREAD_PROCESS_PRIVATE, 0);
    sem_init(&empty, PTHREAD_PROCESS_PRIVATE, QUEUE_SIZE);
}

/******************************************************************************
**************************** MAIN SERVER FUNCTIONS ****************************
******************************************************************************/

/** This function processes the request in the given socket using the given
  * directory as a root path.
  **/
void serve(int hSocket, const string &sDirectory, stringstream &ss)
{
    string sHeaders;
    string sBody;
    string sRequest;
    string sPath;

    ss << "\nParsing Headers......\n";
    vector<char *> headers;
    GetHeaderLines(headers, hSocket, false);
    ss << "DONE\n";

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
        ss << "\nGot from browser:\n";
        PrintVector(headers);
        ss << "\n";

        sRequest = GetRequestedFileName(headers);
        ss << "Requested: " << sRequest << "\n";

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
    ss << sHeaders << "\n";
    string sResponse = sHeaders + sBody;
    write(hSocket, sResponse.c_str(), sResponse.size());

    ss << "\nClosing the socket\n";
    if(close(hSocket) == SOCKET_ERROR)
    {
        ss << "\nCould not close the socket\n";
        cout << ss.str();
    }

    return;
}

void *threadFunc(void *thread_params)
{
    struct thread_params *tp = (struct thread_params*) thread_params;
    long threadID = tp->threadID;
    string sDirectory = tp->dir;
    sem_post(&mutex);

    stringstream ss;
    int hSocket;

    for (;;)
    {
        hSocket = sockqueue.pop();
        ss << "Thread " << threadID; 
        ss << " processing task on socket " << hSocket << endl;
        serve(hSocket, sDirectory, ss);
        ss << "Thread " << threadID << " DONE" << endl;
        cout << ss.str();
        ss.str("");
    }

    return 0;
}

int main(int argc, char* argv[])
{
    int hSocket;
    int hServerSocket;
    struct sockaddr_in Address;
    int nAddressSize = sizeof(struct sockaddr_in);
    int nHostPort;
    int nThreads;
    string sDirectory;

    // Verify arguments
    if (checkArgs(argv, argc))
    {
        nHostPort = atoi(argv[1]);
        nThreads = atoi(argv[2]);
        sDirectory = argv[3];
    }
    else
        return 0;

    // Initialize Semaphores
    initSems();
    
    // Ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);

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
    printf("opened socket as fd (%d) on port (%d) for stream i/o\n", hServerSocket, ntohs(Address.sin_port) );

    printf("Server\n\
      sin_family        = %d\n\
      sin_addr.s_addr   = %d\n\
      sin_port          = %d\n"
      , Address.sin_family
      , Address.sin_addr.s_addr
      , ntohs(Address.sin_port)
      );


    printf("\nMaking a listen queue of %d elements", QUEUE_SIZE);
    if(listen(hServerSocket, QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
        return 0;
    }

    printf("\nCreating %d threads\n", nThreads);
    pthread_t threads[nThreads];
    struct thread_params tp;
    tp.dir = sDirectory;
    for (long threadID = 1; threadID <= nThreads; threadID++)
    {
        sem_wait(&mutex);
        tp.threadID = threadID;
        pthread_create(&threads[threadID], NULL, threadFunc, (void *) &tp);
    }

    for(;;)
    {
        cout << "\nWaiting for a connection\n";
        hSocket = accept(hServerSocket, (struct sockaddr*)&Address, (socklen_t *) &nAddressSize);

        printf("\nGot a connection from %X (%d)\n", Address.sin_addr.s_addr, ntohs(Address.sin_port));
        sockqueue.push(hSocket);
    }
}
