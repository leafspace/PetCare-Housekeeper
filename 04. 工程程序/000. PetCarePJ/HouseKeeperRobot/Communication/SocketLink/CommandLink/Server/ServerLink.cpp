#include "ServerLink.h"

ServerLink::ServerLink()
{
    this->socketServer = new SocketServer();

}

ServerLink::~ServerLink()
{
    delete this->socketServer;
}

bool ServerLink::recvFile()
{
    FILE *fp = NULL;
    char fileName[FILENAME_SIZE] = { 0 };
    int recvLength = 0, writeLength = 0;

    this->createHashCharacters(fileName, FILENAME_SIZE);
    if ((fp = fopen(fileName, "wb")) == NULL) {
        return false;
    }

    do {
        recvLength = this->socketServer->recvMessage();

        if (recvLength < 0) {
            return false;
        }

        uint8_t* recvMessage = this->socketServer->getMessage();
        if (GetCommondType(recvMessage) == Link_EndFile) {
            fwrite(recvMessage, sizeof(uint8_t), 
                strstr((char*)recvMessage, g_commondList[Link_EndFile]) - recvMessage, fp);
            break;
        }

        writeLength = fwrite(recvMessage, sizeof(uint8_t), recvLength, fp);
        if (writeLength < recvLength) {
            return false;
        }
    } while(true);
    return true;
}

void ServerLink::createHashCharacters(char* fileName, const int bufferSize)
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

bool ServerLink::linkClient()
{
    this->linkState = this->socketServer->listenClient();
    if (this->linkState == false) {
        return this->linkState;
    }

    this->linkState = this->socketServer->acceptLink();
    if (this->linkState == false) {
        return this->linkState;
    }
    
    return this->linkState;
}

bool ServerLink::shutdownLink()
{
    this->linkState = false;
    return this->socketServer->closeServerSocket();
}

uint8_t* ServerLink::analyzeCommond()
{
    int length = this->socketServer->recvMessage();
    if (length <= 0) {
        return NULL;
    }

    uint8_t *recvMessage = this->socketServer->getMessage();
    switch(GetCommondType(recvMessage))
    {
        case Link_Commond : return "";
        case Link_File : this->recvFile(); return "";
        case Link_EndFile : return "";
        default: break;
    }

    return recvMessage;
}

void ServerLink::sendCommond(const uint8_t* message, const uint32_t messageSize)
{
    this->socketServer->setMessage(message, messageSize);
    this->socketServer->sendMessage();
}

void ServerLink::sendFile(const char* fileName)
{
    FILE *fp = NULL;
    uint32_t sendSize = 0;
    uint8_t fileBuffer[BUFFER_SIZE] = { 0 };
    if ((fp = fopen(fileName, "rb")) == NULL) {
        return ;
    }

    this->sendCommond(g_commondList[Link_File], strlen(g_commondList[Link_File]));
    while((sendSize = fread(fileBuffer, sizeof(uint8_t), BUFFER_SIZE, fp)) > 0) {
        this->sendCommond(fileBuffer, sendSize);
    }
    
    fclose(fp);
}