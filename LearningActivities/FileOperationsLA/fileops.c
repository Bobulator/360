#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK 1024
#define PATHSIZE 255

void printFile(char *fileName)
{
    char fileBuf[CHUNK];
    FILE *file;
    size_t nread;
    
    file = fopen(fileName, "r");
    if (file)
    {
        while ((nread = fread(fileBuf, 1, sizeof fileBuf, file)) > 0)
	    fwrite(fileBuf, 1, nread, stdout);
	if (ferror(file))
	{
	    printf("ERROR READING FILE");
	}
	fclose(file);
    }
}

int main(int argc, char* argv[])
{
    char *fileName;
    fileName = argv[1];
    printFile(fileName);    
}
