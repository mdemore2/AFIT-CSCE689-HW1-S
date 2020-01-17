#include "TCPServer.h"
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
#include <string>
     
#define TRUE   1  
#define FALSE  0 

/*
Name: Mark Demore, 2d Lt
Course: CSCE689 - Distributed Software Systems
Assignment: HW1 - Cleint/Server Single Process
*/


//socket programming guided by https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/

TCPServer::TCPServer() {

}


TCPServer::~TCPServer() {

}

/**********************************************************************************************
 * bindSvr - Creates a network socket and sets it nonblocking so we can loop through looking for
 *           data. Then binds it to the ip address and port
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::bindSvr(const char *ip_addr, short unsigned int port) {

    int opt = TRUE;   
    int master_socket;

    struct sockaddr_in address;                 
         
    //create a master socket  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
     
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    //type of socket created  
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET,ip_addr,&address.sin_addr);
    address.sin_port = htons( port );   
         
    //bind the socket to localhost port 8888  
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
   
    TCPServer::_serverfd = master_socket;
    TCPServer::_address = address;

}

/**********************************************************************************************
 * listenSvr - Performs a loop to look for connections and create TCPConn objects to handle
 *             them. Also loops through the list of connections and handles data received and
 *             sending of data. 
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::listenSvr() {

    int opt = TRUE;   
    int master_socket = TCPServer::_serverfd;
    struct sockaddr_in address = TCPServer::_address;

    char *message = "Hello from the server! Type 'menu' for options.\n";   

    int addrlen , new_socket , client_socket[30] ,  
          max_clients = 30 , activity, i , valread , sd;   
    int max_sd;   
         
    char buffer[1025];
         
    //set of socket descriptors  
    fd_set readfds;   
         
     
    //initialise all client_socket[] to 0 so not checked  
    for (i = 0; i < max_clients; i++)   
    {   
        client_socket[i] = 0;   
    }

     //try to specify maximum of 3 pending connections for the master socket  
    if (listen(master_socket, 3) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    //accept the incoming connection  
    addrlen = sizeof(address);   
    puts("Waiting for connections ...");   
         
    while(TRUE)   
    {   
        //clear the socket set  
        FD_ZERO(&readfds);   
     
        //add master socket to set  
        FD_SET(master_socket, &readfds);   
        max_sd = master_socket;   
             
        //add child sockets to set  
        for ( i = 0 ; i < max_clients ; i++)   
        {   
            //socket descriptor  
            sd = client_socket[i];   
                 
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                 
            //highest file descriptor number, need it for the select function  
            if(sd > max_sd)   
                max_sd = sd;   
        }   
     
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
             
        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(master_socket, &readfds))   
        {   
            if ((new_socket = accept(master_socket,  
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
             
            //inform user of socket number - used in send and receive commands  
            printf("New connection , socket fd is %d , ip is : %s , port : %d  \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs 
                  (address.sin_port));   

            if( send(new_socket, message, strlen(message), 0) != strlen(message) )   
            {   
                perror("send");   
            }    
                 
            //add new socket to array of sockets  
            for (i = 0; i < max_clients; i++)   
            {   
                //if position is empty  
                if( client_socket[i] == 0 )   
                {   
                    client_socket[i] = new_socket;   
                    printf("Adding to list of sockets as %d\n" , i);   
                         
                    break;   
                }   
            }   
        }   
             
        //else its some IO operation on some other socket 
        for (i = 0; i < max_clients; i++)   
        {   
            sd = client_socket[i];   
                 
            if (FD_ISSET( sd , &readfds))   
            {   
                //Check if it was for closing , and also read the  
                //incoming message  
                if ((valread = read( sd , buffer, 1024)) == 0)   
                {   
                    //Somebody disconnected , get details and print  
                    getpeername(sd , (struct sockaddr*)&address , \ 
                        (socklen_t*)&addrlen);   
                    printf("Host disconnected , ip %s , port %d \n" ,  
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close( sd );   
                    client_socket[i] = 0;   
                }   
                     
                else 
                {   
                    buffer[valread] = '\0';

                    //handle user input
                    processInput(buffer, sd);
                    buffer[0] = '\0'; //clear buffer (potentially unnecessary?)
                }   
            }   
        }   
    }   
}

/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() {

    close( TCPServer::_serverfd );

}

void TCPServer::processInput(const char * buffer, int sd)
{
    std::string input(buffer,strlen(buffer)); //cast to string for comparison
    std::string output = "";

    std::string delimiter = "\n";
    size_t position = 0;
    std::string command;

    while((position = input.find(delimiter)) != std::string::npos)//strip out commands if sent multiple at once
    {
        command = input.substr(0,position);
        input.erase(0,position + delimiter.length());

        if(command == "hello")
        {
            output = "hello user";
        }
        else if(command == "1")
        {
            output = "+ 1 = 2";
        }
        else if(command == "2")
        {
            output = "+ 2 = 4";
        }
        else if(command == "3")
        {
            output = "+ 3 = 6";
        }
        else if(command == "4")
        {
            output = "+ 4 = 8";
        }
        else if(command == "5")
        {
            output = "+ 5 = 10";
        }
        else if(command == "passwd")
        {
            output = "function not yet implemented";
        }
        else if(command == "menu")
        {
            output = "available commands:\n'hello'\n'passwd'\n'menu'\n'1-5'\n'exit'";
        }
        else if(command == "exit")
        {
            output = "goodbye";
        }
        else
        {
            output = "invalid command\ntype 'menu' for list of available commands";
        }

    
        int outlen = output.length() + 1;
        char buffer_out[outlen + 1]; //create buffer

        strcpy(buffer_out,output.c_str());
        buffer_out[outlen+1] = '\0'; //recast and add null
    
        if( send(sd,buffer_out,strlen(buffer_out),0) != strlen(buffer_out) ) //reply to client
        {
            perror("send");
        }
    }

}
