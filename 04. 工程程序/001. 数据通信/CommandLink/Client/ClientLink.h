#include <time.h>
#include "../../BaseSocket/Client/SocketClient.h"

#define FILENAME_SIZE 512
// 1. 接收文件
// 2. 发送文件
// 3. 接收命令
// 4. 发送命令
// 5. 初步解析命令

/*
bool state = false;
SocketClient *socketClinet = new SocketClient("192.168.1.1");
state = socketClient->openClientSocket();
state = socketClient->connectServer();
do
{
    socketClinet->setMessage("Hello", 5);
    socketClient->sendMessage();
    socketClinet->recvMessage();
    uint8_t *recvMessage = socketClient->getMessage();
    // Todo recvMessage
    
} while(true);
state = socketClinet->closeClentSocket();
delete socketClient; 
*/

class ClientLink
{
private:
    bool linkState = false;
    SocketClient *socketClient;

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