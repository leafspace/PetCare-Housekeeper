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

    void initBase(void);
    void initServer(void);
    bool bindServer(void);
    void closeLink(void);
public:
    SocketServer(void);
    ~SocketServer(void);

    bool openServerSocket(void);
    bool listenClient(void);
    bool acceptLink(void);
    int sendMessage(void);
    int recvMessage(void);
    uint8_t* getMessage(void);
    bool closeServerSocket(void);

    void setMessage(const uint8_t* message, uint32_t messageSize);
};