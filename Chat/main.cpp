//
//  main.cpp
//  Chat
//
//  Created by Cho Chong on 2/7/16.
//  Copyright © 2016 Cho Chong. All rights reserved.
//

#include "ChatServer.hpp"
#include <pthread.h>
#include "Record.hpp"
#include "DataStore.hpp"
#include "Threads.hpp"

#define FOREVER while(true)

static ChatServer server;
static DataStore datastore;

void* BroadCastPackets(void *thread_id)
{
    //TODO: use semaphore driven threading or periodic tasking library if available
    FOREVER
    {
        server.BroadCast();
        usleep(50000);
    }
    
    pthread_exit(NULL);
}

void* TransmitPackets(void *thread_id)
{
    FOREVER
    {
        //TODO: need semaphore driven transmits...
        server.Transmit();
        usleep(50000);
    }
    
    pthread_exit(NULL);
}

void* ReceivePackets(void *thread_id)
{
    FOREVER
    {
        server.Receive();
        //TODO: add records to datastore
    }
    
    pthread_exit(NULL);
}

void CreateThreads()
{
    pthread_t threads[NUM_THREADS];
    
    if(pthread_create(&threads[THREAD_RECEIVE], NULL, ReceivePackets, 0))
    {
        printf("Client: Failed to create rx thread\n");
    }

    if(pthread_create(&threads[THREAD_BROADCAST], NULL, BroadCastPackets, 0))
    {
        printf("Client: Failed to create tx thread\n");
    }
    
    if(pthread_create(&threads[THREAD_TRANSMIT], NULL, TransmitPackets, 0))
    {
        printf("Client: Failed to create tx thread\n");
    }
}

int main(int argc, const char * argv[]) {

    if(server.Create())
    {
        CreateThreads();
        
        FOREVER{ sleep(1);}
    }
    
    pthread_exit(NULL);
}
