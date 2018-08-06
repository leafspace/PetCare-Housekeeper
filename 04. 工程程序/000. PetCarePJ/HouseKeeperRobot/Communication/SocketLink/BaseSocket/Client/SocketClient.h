#pragma once

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define BUFFER_SIZE     1024
#define HOST_SIZE       16

/*
  use sample
        bool state = false;
        SocketClient* socketClient = new SocketClient("192.168.1.1");
        state = socketClient->openClientSocket();
        state = socketClient->connectServer();
        do
        {
            socketClient->setMessage((uint8_t*)"Hello", 5);
            socketClient->sendMessage();
            socketClient->recvMessage();
            uint8_t *recvMessage = socketClient->getMessage();
            // Todo recvMessage
            
        } while(true);
        state = socketClient->closeClientSocket();
        delete socketClient;  
*/

class SocketClient
{
private:
    int client_socket = 0;
    uint32_t messageSize = 0;
    uint8_t *fileBuffer = NULL;
    uint8_t *serverHost = NULL;
    uint32_t serverPort = 0;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;

    void initBase(void);
    void initClient(void);
    bool bindClient(void);
    void initServer(void);
    void closeLink(void);
public:
    SocketClient(const char* serverHost, const uint32_t serverPort);
    ~SocketClient(void);

    bool openClientSocket(void);
    bool connectServer(void);
    int sendMessage(void);
    int recvMessage(void);
    uint8_t* getMessage(void);
    bool closeClientSocket(void);
    
    void setMessage(const uint8_t* message, uint32_t messageSize);
    void setServerHost(const char* serverHost);
    void setServerPort(const uint32_t serverPort);
};
