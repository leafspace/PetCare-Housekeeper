#pragma once

#include <time.h>
#include "../CommonCommandType.h"
#include "../../BaseSocket/Client/SocketClient.h"

#define FILENAME_SIZE 512

class ClientLink
{
private:
    bool linkState = false;
    SocketClient *socketClient = NULL;

    bool recvFile(void);
    void createHashCharacters(char* fileName, const int bufferSize);
public:
    ClientLink(const char* ipHost, const uint32_t ipPort);
    ~ClientLink();

    bool linkServer(void);
    bool shutdownLink(void);

    uint8_t* analyzeCommond(void);
    void sendCommond(const uint8_t* message, const uint32_t messageSize);
    void sendFile(const char* fileName);

};