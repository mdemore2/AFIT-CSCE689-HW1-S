#include "TCPClient.h"
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>

/**********************************************************************************************
 * TCPClient (constructor) - Creates a Stdin file descriptor to simplify handling of user input. 
 *
 **********************************************************************************************/

TCPClient::TCPClient() {
}

/**********************************************************************************************
 * TCPClient (destructor) - No cleanup right now
 *
 **********************************************************************************************/

TCPClient::~TCPClient() {

}

/**********************************************************************************************
 * connectTo - Opens a File Descriptor socket to the IP address and port given in the
 *             parameters using a TCP connection.
 *
 *    Throws: socket_error exception if failed. socket_error is a child class of runtime_error
 **********************************************************************************************/

void TCPClient::connectTo(const char *ip_addr, unsigned short port) {

    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    char *hello = "Hello from client"; 
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        perror("socket creation error"); 
        exit(EXIT_FAILURE); 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr)<=0)  
    { 
        perror("invalid address"); 
        exit(EXIT_FAILURE); 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        perror("connection failed"); 
        exit(EXIT_FAILURE); 
    }

    TCPClient::_sock = sock;
    TCPClient::_servaddr = serv_addr;
}

/**********************************************************************************************
 * handleConnection - Performs a loop that checks if the connection is still open, then 
 *                    looks for user input and sends it if available. Finally, looks for data
 *                    on the socket and sends it.
 * 
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::handleConnection() {

    int sock = TCPClient::_sock; 
    int valread_serv,valread_usr; 
    struct sockaddr_in serv_addr = TCPClient::_servaddr; 
    //char *hello = "Hello from client"; 
    char buffer_in[1024] = {0};
    char buffer_out[1024] = {0};
    //std::string user_in  = "";
    std::string user_in(buffer_out,0);
    //send(sock , hello , strlen(hello) , 0 ); 
    //printf("Hello message sent\n"); 
    while(user_in != "exit\n")
    {
        valread_serv = read( sock , buffer_in, 1024); 
        printf("%s\n",buffer_in );

        valread_usr = read(STDIN_FILENO,buffer_out,1024);
        buffer_out[valread_usr] = '\0';

        //strcat(buffer_out,"\n\0");
        send(sock,buffer_out,valread_usr,0);
        user_in = std::string(buffer_out,valread_usr);
        //memset(buffer_in,0,10);
        buffer_in[0] = '\0';
        buffer_out[0] = '\0';
    }
    


}

/**********************************************************************************************
 * closeConnection - Your comments here
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::closeConn() {

    close(TCPClient::_sock);

}


