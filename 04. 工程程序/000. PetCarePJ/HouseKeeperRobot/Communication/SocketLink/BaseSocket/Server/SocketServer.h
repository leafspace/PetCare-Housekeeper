#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_PORT     6666
#define BUFFER_SIZE     1024
#define LISTEN_QUEU     2000

class SocketServer
{
private:
    int server_socket = 0;
    int link_socket = 0;
    uint32_t messageSize = 0;
    uint8_t *fileBuffer = NULL;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;

    void initBase();
    void initServer();
    bool bindServer();
    void closeLink();
public:
    SocketServer();
    ~SocketServer();

    bool openServerSocket();
    bool listenClient();
    inline bool acceptLink();
    inline int sendMessage();
    inline int recvMessage();
    uint8_t* getMessage();
    bool closeServerSocket();

    void setMessage(const uint8_t* message, uint32_t messageSize);
    void setServerPort(const uint32_t serverPort);
    void setBufferSize(const uint32_t bufferSize);
};