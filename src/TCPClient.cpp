#include "TCPClient.h"
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>

/*
Name: Mark Demore, 2d Lt
Course: CSCE689 - Distributed Software Systems
Assignment: HW1 - Cleint/Server Single Process
*/

//socket programming guided by https://www.geeksforgeeks.org/socket-programming-cc/

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

    int sock = 0; 
    struct sockaddr_in serv_addr;

    //create socket 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        perror("socket creation error"); 
        exit(EXIT_FAILURE); 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port); 
       
    // Convert IP addresses from text to binary 
    if(inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr)<=0)  
    { 
        perror("invalid address"); 
        exit(EXIT_FAILURE); 
    } 
   
    //connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        perror("connection failed"); 
        exit(EXIT_FAILURE); 
    }

    //add to private vars to share with other functions
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
    struct sockaddr_in serv_addr = TCPClient::_servaddr; 
    
    char buffer_in[1024] = {0};
    char buffer_out[1024] = {0};
    int valread_serv,valread_usr; 

    int flag = 1;
   
    std::string user_in(buffer_out,0);
   
    while(flag > 0)
    {
        valread_serv = read( sock , buffer_in, 1024); //read new msg from server
        buffer_in[valread_serv] = '\0'; //add null terminator
        printf("%s\n\n",buffer_in );

        valread_usr = read(STDIN_FILENO,buffer_out,1024); //read user input
        buffer_out[valread_usr] = '\0'; //add null terminator

        
        if( send(sock,buffer_out,valread_usr,0) != strlen(buffer_out) ) //send msg
        {
            perror("send");
        }
        
        user_in = std::string(buffer_out,valread_usr); //cast to string for comparison

        std::string delimiter = "\n";
        size_t position = 0;
        std::string command;

        while((position = user_in.find(delimiter)) != std::string::npos)//strip out commands if sent multiple at once
        {
            command = user_in.substr(0,position);
            user_in.erase(0,position + delimiter.length());

            if(command == "exit")
            {
                flag = -1;
            }
        
        }
        
        buffer_in[0] = '\0'; //clear buffers (potentially unnecessary)
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


