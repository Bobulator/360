#include <string.h>
#include <vector>             // stl vector
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <dirent.h>

using namespace std;

#define MAX_MSG_SZ      1024

// Determine if the character is whitespace
bool isWhitespace(char c)
{ switch (c)
    {
        case '\r':
        case '\n':
        case ' ':
        case '\0':
            return true;
        default:
            return false;
    }
}

// Strip off whitespace characters from the end of the line
void chomp(char *line)
{
    int len = strlen(line);
    while (isWhitespace(line[len]))
    {
        line[len--] = '\0';
    }
}

// Read the line one character at a time, looking for the CR
// You dont want to read too far, or you will mess up the content
char * GetLine(int fds)
{
    char tline[MAX_MSG_SZ];
    char *line;
    
    int messagesize = 0;
    int amtread = 0;
    while((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ)
    {
        if (amtread >= 0)
            messagesize += amtread;
        else
        {
            perror("Socket Error is:");
            fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
            exit(2);
        }
        //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
        if (tline[messagesize - 1] == '\n')
            break;
    }
    tline[messagesize] = '\0';
    chomp(tline);
    line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
    strcpy(line, tline);
    //fprintf(stderr, "GetLine: [%s]\n", line);
    return line;
}
    
// Change to upper case and replace with underlines for CGI scripts
void UpcaseAndReplaceDashWithUnderline(char *str)
{
    int i;
    char *s;
    
    s = str;
    for (i = 0; s[i] != ':'; i++)
    {
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] = 'A' + (s[i] - 'a');
        
        if (s[i] == '-')
            s[i] = '_';
    }
    
}


// When calling CGI scripts, you will have to convert header strings
// before inserting them into the environment.  This routine does most
// of the conversion
char *FormatHeader(char *str, const char *prefix)
{
    char *result = (char *)malloc(strlen(str) + strlen(prefix));
    char* value = strchr(str,':') + 1;
    UpcaseAndReplaceDashWithUnderline(str);
    *(strchr(str,':')) = '\0';
    sprintf(result, "%s%s=%s", prefix, str, value);
    return result;
}

// Get the header lines from a socket
//   envformat = true when getting a request from a web client
//   envformat = false when getting lines from a CGI program

void GetHeaderLines(vector<char *> &headerLines, int skt, bool envformat)
{
    // Read the headers, look for specific ones that may change our responseCode
    char *line;
    char *tline;
    
    tline = GetLine(skt);
    while(strlen(tline) != 0)
    {
        if (strstr(tline, "Content-Length") || 
            strstr(tline, "Content-Type"))
        {
            if (envformat)
                line = FormatHeader(tline, "");
            else
                line = strdup(tline);
        }
        else
        {
            if (envformat)
                line = FormatHeader(tline, "HTTP_");
            else
            {
                line = (char *)malloc((strlen(tline) + 10) * sizeof(char));
                sprintf(line, "HTTP_%s", tline);                
            }
        }
        //fprintf(stderr, "Header --> [%s]\n", line);
        
        headerLines.push_back(line);
        free(tline);
        tline = GetLine(skt);
    }
    free(tline);
}

// Retrieves the request file from a vector of headers.
// This assumes that the string following the GET command
// will be the requested file.
string getRequestedFileName(vector<char *> &headerLines)
{
    vector<string> elems;
    stringstream ss(headerLines[0]);
    string item;
    
    while(getline(ss, item, ' '))
    {
        if (!item.empty())
            elems.push_back(item);
    }

    return elems[1];
}

void printVector(const vector<char *> v)
{
    for (int i = 0; i < (int) v.size(); ++i)
    {
        printf("%s\n", v[i]);
    }
}

int readFileOrDirectory(const char *name, char *buff, int buffSize)
{
    struct stat filestat;

    if (stat(name, &filestat))
    {
        return -1;
    }

    else if (S_ISREG(filestat.st_mode))
    {
        //cout << argv[1] << " is a regular file" << endl;
        //cout << "file size = " << filestat.st_size << endl;
        FILE *fp = fopen(name, "r");
        fread(buff, buffSize, 1, fp);
        //cout << "FILE " << endl << buff << endl;
        fclose(fp);

        return 1;
    }

    else if (S_ISDIR(filestat.st_mode))
    {
        //cout << argv[1] << " is a directory" << endl;
        DIR *dirp;
        struct dirent *dp;

        dirp = opendir(name);
        int written = 0;
        while ((dp = readdir(dirp)) != NULL && written < buffSize)
        {
            buff = dp->d_name;
            buff += strlen(dp->d_name);
            char tmp[] = "\n";
            buff = tmp;
            buff++;
            written += strlen(dp->d_name) + 1;
            //cout << "name " << dp->d_name << endl;
        }
        (void)closedir(dirp);

        return 2;
    }

    return 0;
}
