#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <cstdlib>

using namespace std;

main(int argc, char **argv)
{
	struct stat filestat;

	if (stat(argv[1], &filestat))
	{
		cout << "ERROR in stat\n";
	}

	if (S_ISREG(filestat.st_mode))
	{
		cout << argv[1] << " is a regular file" << endl;
		cout << "file size = " << filestat.st_size << endl;
		FILE *fp = fopen(argv[1], "r");
		char *buff = (char *)malloc(filestat.st_size);
		fread(buff, filestat.st_size, 1, fp);
		cout << "FILE " << endl << buff << endl;
		free(buff);
		fclose(fp);
	}

	if (S_ISDIR(filestat.st_mode))
	{
		cout << argv[1] << " is a directory" << endl;
		DIR *dirp;
		struct dirent *dp;

		dirp = opendir(argv[1]);
		while ((dp = readdir(dirp)) != NULL)
		{
			cout << "name " << dp->d_name << endl;
		}
		(void)closedir(dirp);
	}
}
