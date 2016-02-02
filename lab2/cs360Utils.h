#include <string.h>
#include <vector>             // stl vector
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <dirent.h>

using namespace std;

#define MAX_MSG_SZ      1024

// Determine if the character is whitespace
bool isWhitespace(char c)
{
    switch (c)
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

        if (amtread > 0)
        {
            messagesize += amtread;
        }
        else if( amtread == 0 )
        {
            break;
        }
        else
        {
            perror("Socket Error is:");
            fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
            exit(2);
        }
        //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
        if (tline[messagesize - 1] == '\n')
        {
            break;
        }
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
char *FormatHeader(char *str, char *prefix)
{
    char *result = (char *)malloc(strlen(str) + strlen(prefix));
    char* value = strchr(str,':') + 2;
    UpcaseAndReplaceDashWithUnderline(str);
    *(strchr(str,':')) = '\0';
    sprintf(result, "%s%s=%s", prefix, str, value);
    return result;
}

// Get the header lines from a socket
//   envformat = true when getting a request from a web client
//   envformat = false when getting lines from a CGI program

void GetHeaderLines(std::vector<char *> &headerLines, int skt, bool envformat)
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
                line = FormatHeader(tline, const_cast<char*>( "" ) );
            else
                line = strdup(tline);
        }
        else
        {
            if (envformat)
                line = FormatHeader(tline, const_cast<char*>( "HTTP_" ) );
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

void PrintVector(const vector<char *> v)
{
    for (int i = 0; i < (int) v.size(); ++i)
    {
        printf("%s\n", v[i]);
    }
}

string GetRequestedFileName(vector<char *> &headerLines)
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

string GetExt(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";

    if (strcmp(dot, ".txt") == 0)
        return "text/plain";
    else if (strcmp(dot, ".jpg") == 0)
        return "image/jpg";
    else if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    else
        return "text/html";
}

string FileToString(const char *filePath, struct stat &filestat)
{
    printf("Opening and reading ");
    string fileExt = GetExt(filePath);
    FILE *fp;
    char *fBuff;

    if (fileExt.compare(0, 5, "image") == 0)
    {
        printf("image.....");
        fp = fopen(filePath, "rb");
    }
    else
    {
        printf("file.....");
        fp = fopen(filePath, "r");
    }

    fBuff = (char *) malloc(filestat.st_size + 1);
    int readAmt = fread(fBuff, 1, filestat.st_size + 1, fp);
    string str(fBuff, filestat.st_size);

    fclose(fp);
    free(fBuff);
    printf("DONE\nRead: %d\nExpected: %d\n", readAmt, (int) filestat.st_size);

    return str;
}

string DirectoryToHtml(const char *dir)
{
    stringstream ss;
    ss << "<html>\n"
       << "<body>\n"
       << "<ol>\n";

    DIR *dirp;
    struct dirent *dp;
    printf("\nTRYING TO OPEN THIS DIRECTORY: %s\n", dir);
    dirp = opendir(dir);
    while ((dp = readdir(dirp)) != NULL)
        ss << "<li><a href=" << dp->d_name << ">/" << dp->d_name << "</a>\n";
    (void) closedir(dirp);

    ss << "</ol>\n"
       << "</body>\n"
       << "</html>";

    return ss.str();
}
