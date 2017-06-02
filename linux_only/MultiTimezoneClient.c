//
//  MultiTimezoneClient
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

int main(int argc, char * argv[])
{
    const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE + 1];
    //use BUFFER_SIZE + 1 so there is an extra byte available for \0 (null terminator string)
    ssize_t bytes_read;

    int socketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(socketHandle <= -1)
    {
        printf("[Fatal Error]: Socket Handle is invalid, aborting.\n");
        return -1;
    }
    // internet domain (ipv4), sock_stream (so not using packets), and TCP protocol
    struct sockaddr_in sockAddress;
    //ensure the struct is empty (no noisy values)
    memset(&sockAddress, 0, sizeof(sockAddress));

    sockAddress.sin_family = AF_INET;
    sockAddress.sin_port = htons(kMULTITIMEZONEPORT);

    //ensure endianness is valid
    struct hostent* dnsAddress;
    dnsAddress = gethostbyname(argv[1]);
    //argv[1] should contain a valid DNS lookup (e.g www.youtube.com), ipv4 address e.g 127.0.0.1 works too
    
    memcpy(&sockAddress.sin_addr, dnsAddress->h_addr, dnsAddress->h_length);

    connect(socketHandle, (struct sockaddr*)&sockAddress, sizeof(sockAddress));
    //cast to sockaddr to prevent annoying warnings
    //sockAddress is a struct containingf the ip address and port.

    //todo: read/write stuff
    unsigned int expecting = 1;
    while(expecting == 1)
    {
        bytes_read = read(socketHandle, buffer, BUFFER_SIZE);
        buffer[BUFFER_SIZE] = 0;
        int i;
        if(bytes_read <= 0)
            break;
        for(i = 0; i < bytes_read; i++)
        {
            if(buffer[i] == 10)
            {
                //  We found a line break, so this is the end of the message.
                expecting = 0;
            }
        }
        printf("%s", buffer);
    }
    //now we, as the client, want to send commands for time zones (e.g 'PST') or the word 'CLOSE'
    int j;
    if(argc > 2)
    {
        j = 2;
        while(j < argc)
        {
            //printf("Requesting time via '%s':\n", argv[j]);
            char* cmd = (char*)malloc(sizeof(argv[j] + 1));
            strcpy(cmd, argv[j]);
            printf("%s\n", cmd);
            strcat(cmd, "\n");
            write(socketHandle, cmd, strlen(cmd));
            free(cmd);
            expecting = 1;
            while(expecting == 1)
            {
                bytes_read = read(socketHandle, buffer, BUFFER_SIZE);
                buffer[BUFFER_SIZE] = 0;
                int i;
                if(bytes_read <= 0)
                    break;
                for(i = 0; i < bytes_read; i++)
                {
                    if(buffer[i] == 10)
                    {
                        //  We found a line break, so this is the end of the message.
                        expecting = 0;
                    }
               }
               printf("%s", buffer);
            }
            j++;
        }
        printf("\n");
    }


    close(socketHandle);
    return 0;
}
