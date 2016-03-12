//
//  ChatServer.hpp
//  Chat
//
//  Created by Cho Chong on 2/7/16.
//  Copyright © 2016 Cho Chong. All rights reserved.
//

#ifndef ChatServer_hpp
#define ChatServer_hpp

#include <stdio.h>
#include <sys/types.h>
#include "PacketDriver.hpp"

class ChatServer
{
public:
    ChatServer();
    ~ChatServer();
    
    
    int Create();
    void Receive();
    void Transmit();
    void BroadCast();
private:
    void Initialize();
    static void sigchld_handler(int s);
    void *get_in_addr(struct sockaddr *sa);
    ChatLib::PacketDriver StreamPacketDriver;
    int listener;
    fd_set MasterFds;
    int FdMax;
};

#endif /* ChatServer_hpp */
