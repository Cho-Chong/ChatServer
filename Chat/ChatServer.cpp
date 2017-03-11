//
//  ChatServer.cpp
//  Chat
//
//  Created by Cho Chong on 2/7/16.
//  Copyright © 2016 Cho Chong. All rights reserved.
//

#include "ChatServer.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "PacketWrapper.hpp"
#include "Record.hpp"

#define SERVER_PORT ("3501")
#define MAX_PENDING_CONNECTIONS (10)
#define SERVER_MSG ("Hello, World!")
static const unsigned int RECEIVE_TIMEOUT = 5;

ChatServer::ChatServer(ChatLib::Interface::IIODevice* iodevice)
{
    StreamPacketDriver = new ChatLib::PacketDriver(iodevice);
}

ChatServer::~ChatServer()
{
    Initialize();
}

void ChatServer::Initialize()
{
    FD_ZERO(&this->MasterFds);
}

void ChatServer::sigchld_handler(int s)
{
    int saved_errno = errno;
    
    while(waitpid(-1, NULL, WNOHANG) > 0);
    
    errno = saved_errno;
}

void* ChatServer::get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    else
    {
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
}

int ChatServer::Create()
{
    int sockfd = 0;
    struct addrinfo hints, *servinfo, *cur_sock;
    int yes = 1;
    int rv;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    Initialize();
    
    if ( (rv = getaddrinfo(NULL, SERVER_PORT, &hints, &servinfo) != 0))
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 0;
    }
    else
    {
        for(cur_sock = servinfo; cur_sock != NULL; cur_sock = cur_sock->ai_next)
        {
            if( (sockfd = socket(cur_sock->ai_family, cur_sock->ai_socktype, cur_sock->ai_protocol)) == -1)
            {
                perror("server: socket");
                continue;
            }
            
            if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,  &yes, sizeof(int)) == -1)
            {
                perror("setsockopt");
                return 0;
            }
            
            if(bind(sockfd, cur_sock->ai_addr, cur_sock->ai_addrlen) == -1)
            {
                close(sockfd);
                perror("server: bind");
                continue;
            }
            
            break;
        }
        
        freeaddrinfo(servinfo);
        
        if(cur_sock == NULL)
        {
            fprintf(stderr, "server: failed to bind\n");
            return 0;
        }
        
        if(listen(sockfd, MAX_PENDING_CONNECTIONS) == -1)
        {
            perror("listen");
            return 0;
        }
        else
        {
            FD_SET(sockfd, &this->MasterFds);
        }
        
        listener = sockfd;
        FdMax = listener;
        
        return 1;
    }
}

void ChatServer::Receive()
{
    struct sockaddr_storage remoteaddr; // client address
    fd_set read_fds = this->MasterFds;
    char remoteIP[INET_ADDRSTRLEN];
    struct timeval time;
    
    //periodically check for disconnected clients
    time.tv_sec = RECEIVE_TIMEOUT;
    time.tv_usec = 0;
    
    auto socket_status = select(FdMax+1, &read_fds, nullptr, nullptr, &time);
    
    if(socket_status == -1)
    {
        perror("select");
    }
    else
    {
        for(int i = 0; i <= FdMax; i++)
        {
            if(FD_ISSET(i, &read_fds))
            {
                if(i == listener)
                {
                    socklen_t addrlen = sizeof(remoteaddr);
                    int newfd = accept(listener,
                                       (struct sockaddr *)&remoteaddr,
                                       &addrlen);
                    
                    if (newfd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        FD_SET(newfd, &this->MasterFds);
                        
                        if(newfd > FdMax)
                        {
                            FdMax = newfd;
                        }
                        
                        printf("selectserver: new connection from %s on socket %d\n",
                               inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN), newfd);
                    }
                }
                else
                {
                    if(StreamPacketDriver->RecvPacket(i) == ChatLib::PACKET_FAILURE)
                    {
                        FD_CLR(i, &this->MasterFds);
                        
                        StreamPacketDriver->CleanSocket(i);
                        printf("Disconnecting from socket %d\n", i);
                        //TODO: reset fdmax
                    }
                }
            }
        }
    }
}

void ChatServer::BroadCast()
{
    ChatLib::PACKET_T packet;
    packet.header.length = 0;
    
    do
    {
        StreamPacketDriver->DequeuePacket(packet);
        
        if(packet.header.length > 0)
        {
            packet.header.source = ChatLib::NODE_SERVER;
            packet.header.dest = ChatLib::NODE_CLIENT;
            
            // we got some data from a client
            for(int j = 0; j <= FdMax; j++) {
                if (FD_ISSET(j, &this->MasterFds) && j != listener) {
                    StreamPacketDriver->QueuePacket(j, packet);
                }
            }
        }
    }
    while(packet.header.length > 0);
}

void ChatServer::Transmit()
{
    // we got some data from a client
    for(int j = 0; j <= FdMax; j++) {
        // except the listener and ourselves
        if (FD_ISSET(j, &this->MasterFds) && j != listener) {
            StreamPacketDriver->SendPackets(j);
        }
    }
}