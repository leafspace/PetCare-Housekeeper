#pragma once

#include <time.h>
#include "../../BaseSocket/Server/SocketServer.h"

#define FILENAME_SIZE 512

class ServerLink
{
private:
    bool linkState = false;
    SocketServer *socketServer = NULL;

    const uint8_t commondListSize = 2;
    const char *commondList[2] = { "Link-file", "Link-commond" };

    void createHashCharacters(char* fileName, const int bufferSize);
public:
    ServerLink();
    ~ServerLink();

    bool linkClient();
    bool shutdownLink();

    uint8_t* analyzeCommond();
    bool recvFile();
    void sendCommond(const uint8_t* message, const uint32_t messageSize);
    void sendFile(const char* fileName);
};
