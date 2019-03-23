#pragma once

#include <time.h>
#include "../CommonCommandType.h"
#include "../../BaseSocket/Server/SocketServer.h"

#define FILENAME_SIZE 512

class ServerLink
{
private:
    bool linkState = false;
    SocketServer *socketServer = NULL;

    bool recvFile(void);
    void createHashCharacters(char* fileName, const int bufferSize);
public:
    ServerLink(void);
    ~ServerLink(void);

    bool linkClient(void);
    bool shutdownLink(void);

    uint8_t* analyzeCommond(void);
    void sendCommond(const uint8_t* message, const uint32_t messageSize);
    void sendFile(const char* fileName);
};
