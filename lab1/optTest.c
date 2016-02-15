#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[])
{
	extern char *optarg;
	int c = 1;
	int times_to_download = 1; 
	int err = 0;
	bool debug = false;

	while((c = getopt(argc, argv, "c:d")) != -1)
	{
		switch(c)
		{
			case 'c':
				times_to_download = atoi(optarg);
				break;
			case 'd':
				debug = true;
			case '?':
				err = 1;
				break;
				
		}
	}

	string host = argv[optind];
	int port = atoi(argv[optind + 1]);

	cout << "HOST = " << host << endl;
	cout << "PORT = " << port << endl;
	cout << "OPTIONS = " << times_to_download << " " << debug << endl;
}