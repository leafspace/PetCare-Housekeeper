#include "ClientLink.h"

ClientLink::ClientLink(const char* ipHost)
{
    this->socketClient = new SocketClient(ipHost);

}

ClientLink::~ClientLink()
{
    delete this->socketClient;
}

void ClientLink::createHashCharacters(char* fileName, const int bufferSize)
{
    struct tm *nTime;
    time_t timet;
    time(&timet);
    nTime = localtime(&timet);
    sprintf(fileName, "%4d%02d%02d%02d%02d%02d", nTime->tm_year + 1900, 
        nTime->tm_mon + 1, nTime->tm_mday, nTime->tm_hour, nTime->tm_min, nTime->tm_sec);

    char metachar[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    srand((unsigned)time(NULL));
    for (int i = 14; i < 128 + 14; i++) {
        fileName[i] = metachar[rand() % 62]; 
    }
    fileName[128 + 14] = '\0';
}

bool ClientLink::linkServer()
{
    this->linkState = this->socketClient->openClientSocket();
    if (this->linkState == false) {
        return this->linkState;
    }

    this->linkState = this->socketClient->connectServer();
    if (this->linkState == false) {
        return this->linkState;
    }

    return this->linkState;
}

bool ClientLink::linkServer(const char* ipHost)
{
    delete this->socketClient;
    this->socketClient = new SocketClient(ipHost);
}

bool ClientLink::shutdownLink()
{
    this->linkState = false;
    return this->socketClient->closeClientSocket();
}

uint8_t* ClientLink::analyzeCommond()
{
    int length = this->socketClient->recvMessage();
    if (length <= 0) {
        return NULL;
    }

    uint8_t *recvMessage = this->socketClient->getMessage();
    for (uint8_t i = 0; i < this->commondListSize; ++i) {
        if (strstr((char*)recvMessage, this->commondList[i])) {
            switch(i)
            {
                case 0 : this->recvFile(); break;
                case 1 : break;
                default: break;
            }

            break;
        }
    }

    return recvMessage;
}

bool ClientLink::recvFile()
{
    FILE *fp = NULL;
    char fileName[FILENAME_SIZE] = { 0 };
    int recvLength = 0, writeLength = 0;

    this->createHashCharacters(fileName, FILENAME_SIZE);
    if ((fp = fopen(fileName, "wb")) == NULL) {
        return false;
    }

    while(recvLength = this->socketClient->recvMessage()) {
        if (recvLength < 0) {
            return false;
        }

        uint8_t* recvMessage = this->socketClient->getMessage();
        writeLength = fwrite(recvMessage, sizeof(uint8_t), recvLength, fp);  
        if (writeLength < recvLength) {
            return false;
        }
    }
    return true;
}

void ClientLink::sendCommond(const uint8_t* message, const uint32_t messageSize)
{
    this->socketClient->setMessage(message, messageSize);
    this->socketClient->sendMessage();
}

void ClientLink::sendFile(const char* fileName)
{
    FILE *fp = NULL;
    uint32_t sendSize = 0;
    uint8_t fileBuffer[BUFFER_SIZE] = { 0 };
    if ((fp = fopen(fileName, "rb")) == NULL) {
        return ;
    }

    while((sendSize = fread(fileBuffer, sizeof(uint8_t), BUFFER_SIZE, fp)) > 0) {
        this->sendCommond(fileBuffer, sendSize);
    }
}