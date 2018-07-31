#pragma once

#include <time.h>
#include "../../BaseSocket/Client/SocketClient.h"

#define FILENAME_SIZE 512

class ClientLink
{
private:
    bool linkState = false;
    SocketClient *socketClient = NULL;

    const uint8_t commondListSize = 2;
    const char *commondList[2] = { "Link-file", "Link-commond" };

    void createHashCharacters(char* fileName, const int bufferSize);
public:
    ClientLink(const char* ipHost);
    ~ClientLink();

    bool linkServer();
    bool linkServer(const char* ipHost);
    bool shutdownLink();

    uint8_t* analyzeCommond();
    bool recvFile();
    void sendCommond(const uint8_t* message, const uint32_t messageSize);
    void sendFile(const char* fileName);

};