#pragma once

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace std;

#define BUFFER_SIZE     1024
#define HOST_SIZE       16

class SocketClient
{
private:
    int client_socket = 0;
    uint32_t messageSize = 0;
    uint8_t *fileBuffer = NULL;
    uint8_t serverHost[16] = { 0 };
    uint32_t serverPort = 0;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;

    void initBase(void);
    void initClient(void);
    bool bindClient(void);
    void initServer(void);
    void closeLink(void);
public:
    SocketClient(const uint8_t* serverHost, const uint32_t serverPort);
    ~SocketClient(void);

    bool openClientSocket(void);
    bool connectServer(void);
    int sendMessage(void);
    int recvMessage(void);
    uint8_t* getMessage(void);
    bool closeClientSocket(void);
    
    void setMessage(const uint8_t* message, uint32_t messageSize);
    void setServerHost(const uint8_t* serverHost);
    void setServerPort(const uint32_t serverPort);
};
