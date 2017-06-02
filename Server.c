#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <time.h>

#define kDAYTIMEPORT       1313
#define kMULTITIMEZONEPORT 1414

int main(int argc, const char** argv)
{
    unsigned int clientSize;
    int clientHandle;
    int socketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketHandle <= -1)
    {
        printf("[Fatal Error]: Socket Handle is invalid, aborting.\n");
        return -1;
    }
    struct sockaddr_in server, client;
    memset(&server, 0, sizeof(server));
    //memset(&client, 0, sizeof(client));
    server.sin_family = AF_INET;
    server.sin_port = htons(kMULTITIMEZONEPORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if(bind(socketHandle, (struct sockaddr*) &server, sizeof(server)) <= -1)
    {
        printf("[Fatal Error]: Socket Handle could not be bound to server, aborting.\n");
        return -1;
        //bad, couldnt bind socket. print error message and run away
    }
    listen(socketHandle, 3);    //queue length of 3 because there's no reason to have many
    while(1)
    {
        char* buffer;
        time_t curTime;
        //curTime holds the number of seconds passed since January 1, 1970.
        //seconds elapsed at Jan 1 1970 = ?
        //days passed = 1970(365) = 719050
        //seconds passed between year 0 and epoch = 719050(24 * 3600)
        curTime += (719050L * 24L * 3600L);
        time(&curTime);
        buffer = ctime(&curTime);
        //buffer always has a \n at the END of it, so we can just made the 2nd last character nullterm to remove the \n.
        buffer[strlen(buffer)-1] = 0;
        strcat(buffer, "-GMT\r\n");
        //add the -GMT and then CRLF

        clientSize = sizeof(client);
        clientHandle = accept(socketHandle, (struct sockaddr*) &client, &clientSize);
        if(clientHandle <= -1)
        {
            printf("[Fatal Error]: Detected malformed client handle, aborting.\n");
            close(clientHandle);
            break;
        }
        //Connection should be made by now
        //todo: read/write stuff
        write(clientHandle, buffer, strlen(buffer));
        close(clientHandle);
    }
    close(socketHandle);
    return 0;
}
