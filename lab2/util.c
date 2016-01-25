#include "util.h"
#include <stdio.h>
#include <string.h>

void getFileName(char *request, char *rBuffer)
{
	char temp[strlen(request)];
	strncpy(temp, request, strlen(request));

	const char *get = "GET";
	char *token = strtok(temp, " \r\n");
	while (strncmp(get, token, 3) != 0)
		token = strtok(NULL, " ");

	token = strtok(NULL, " ");

	if (sizeof rBuffer >= sizeof token)
		strcpy(rBuffer, token);
	else 
		//ERROR

	printf("buffer %s\n", rBuffer);
}

// char *strnstr(const char *haystack, const char *needle, size_t len)
// {
//         int i;
//         size_t needle_len;

//         /* segfault here if needle is not NULL terminated */
//         if (0 == (needle_len = strlen(needle)))
//                 return (char *)haystack;

//         for (i=0; i<=(int)(len-needle_len); i++)
//         {
//                 if ((haystack[0] == needle[0]) &&
//                         (0 == strncmp(haystack, needle, needle_len)))
//                         return (char *)haystack;

//                 haystack++;
//         }
//         return NULL;
// }