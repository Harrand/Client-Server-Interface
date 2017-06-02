//
//  MultiTimezoneServer
//
//  Created by Harry Hollands (from Skeleton provided)
//  Username: psyhsh
//

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

int printCurTime(int clientHandle, char* cmd)
{
    char* buffer;
    time_t now, curTime;
    char farewell[5] = "BYE\r\n", err[7] = "ERROR\r\n";
    time(&now);

    if(strcmp(cmd, "CLOSE\n") == 0)
    {
         //close the current client and break the loop so the server closes.
        write(clientHandle, farewell, strlen(farewell));
        close(clientHandle);
        return -1;
    }
    else if(strcmp(cmd, "PST\n") == 0)
    {
        curTime = now - (8*3600);
    }
    else if(strcmp(cmd, "MST\n") == 0)
    {
        curTime = now - (7*3600);
    }
    else if(strcmp(cmd, "CST\n") == 0)
    {
        curTime = now - (6*3600);
    }
    else if(strcmp(cmd, "EST\n") == 0)
    {
        curTime = now - (5*3600);
    }
    else if(strcmp(cmd, "GMT\n") == 0)
    {
        curTime = now - 0;
    }
    else if(strcmp(cmd, "CET\n") == 0)
    {
        curTime = now + (1*3600);
    }
    else if(strcmp(cmd, "MSK\n") == 0)
    {
        curTime = now + (3*3600);
    }
    else if(strcmp(cmd, "JST\n") == 0)
    {
        curTime = now + (9*3600);
    }
    else if(strcmp(cmd, "AEST\n") == 0)
    {
        curTime = now + (10*3600);
    }
    else
    {
        write(clientHandle, err, strlen(err));
        return 0;
    }
    buffer = ctime(&curTime);
    //buffer always has a \n at the END of it, so we can just made the 2nd last character nullterm to remove the \n.
    buffer[strlen(buffer)-1] = 0;
    strcat(buffer, "-");
    strcat(buffer, cmd);
    //strcat(buffer, "\r\n");
    //strcat(buffer, "-GMT\r\n");
    //add the -GMT and then CRLF
    write(clientHandle, buffer, strlen(buffer));
    return 0;
}

int main(int argc, const char * argv[])
{
    unsigned int clientSize;
    ssize_t bytes_read;
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
        const unsigned int INPUT_SIZE = 512;
        char input[INPUT_SIZE];
        //was here before
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
        printCurTime(clientHandle, "GMT\n");
        char cmd[INPUT_SIZE];
        int endOfCmd = 0;
        memset(&cmd[0], 0, sizeof(cmd));
        while(strcmp(cmd, "CLOSE") != 0)    //while the command we're currently gathering isnt CLOSE...
        {
            bytes_read = read(clientHandle, input, INPUT_SIZE); // read whatever's next.
            //input[INPUT_SIZE] = 0;  //add in the null-terminator
            //printf("Added null-terminator.\n");
            int i;
            if(bytes_read <= 0)
                break;  //if we didnt actually read anything then just end the loop.
            for(i = 0; i < bytes_read; i++) //for each char we just read, if any of them are \n, then stop reading
            {
                if(input[i] == 10)
                {
                    endOfCmd = 1;
                    break;
                }
            }
            strcat(cmd,input);
            if(endOfCmd == 1)
            {
                if(printCurTime(clientHandle, cmd) == -1)
                {
                    close(socketHandle);
                    return 0;
                }
                memset(&cmd[0], 0, sizeof(cmd));
                memset(&input[0], 0, sizeof(input));
                endOfCmd = 0;
            }
        }
    }
    close(socketHandle);
    return 0;
}
