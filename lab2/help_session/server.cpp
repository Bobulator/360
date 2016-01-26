#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "serverUtils.cpp"

void serv(int conn_sock)
{
	// process GET request
	// parse http headers
	std::vector<char*> headers;
	GetHeaderLines(headers, conn_sock, false);
	
	std::cout << headers[0] << endl;	
	
	// serve
	std::string msg = "HTTP/1.1 2-- OK/r/nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nhey guy";
	int n = write(conn_sock, msg.c_str(), msg.length());
	if (n == -1)
	{
		std::cout << "write failed" << endl;
		exit(0);
	}
}

int main(int argc, char* argv[])
{
	// Parse input params
	int port_num = atoi(argv[1]);
	
	// Directory to look stuff in
	std::string content_folder = argv[2];

	// socket stuff
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		std::cout << "opening socket broke" << std::endl;
	}

	// socket address
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_num);
	serv_addr.sin_addr.s_addr = INADDR_ANY; 
		

	// bind
	int sock_len = sizeof(struct sockaddr_in);
	
	for(;;)
	{
	
		int bind_return = bind(fd, (sockaddr*) &serv_addr, (socklen_t) sizeof(serv_addr)); 
		
		// check the bind value
		if (bind_return == -1)
		{
			std::cout << "bind failed" << endl;
			exit(0);
		}
		// listen?
		int list_return = listen(fd, 1000);
	
		// check the listen return value
		if (list_return == -1)
		{
			std::cout << "list failed" << endl;
			exit(0);
		}
	
		// accept connection
		// make empty sockaddr to fill in when 
		// a connection is made
		struct sockaddr_in cli_addr;
		int conn_sock = accept(fd, (sockaddr*) &cli_addr, (socklen_t*) &sock_len);
	
		// check accept return value
		if (conn_sock == -1)
		{
			std::cout << "accept failed" << endl;
			exit(0);	
		}
		
		
		serv(conn_sock);	
	}
}
