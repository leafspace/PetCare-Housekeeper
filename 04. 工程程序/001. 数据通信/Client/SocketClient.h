#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_PORT     6666
#define BUFFER_SIZE     1024

/* Todo 待确认(UBUNTU下) */
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>
/* Todo 待确认(UBUNTU下) */

/*
  use sample
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

class SocketClient
{
private:
    int client_socket = 0;
    uint8_t *fileBuffer = NULL;
    uint8_t *serverHost = NULL;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;

    void initBase();
    void initClient();
    bool bindClient();
    void initServer();
    void closeLink();
public:
    SocketClient(const char* serverHost);
    ~SocketClient();

    bool openClientSocket();
    bool connectServer();
    inline int sendMessage();
    inline int recvMessage();
    uint8_t* getMessage();
    bool closeClientSocket();
    
    void setMessage(uint8_t* message, uint32_t messageSize);
    void setServerHost(const char* serverHost);
    void setServerPort(const uint32_t serverPort);
    void setBufferSize(const uint32_t bufferSize);
};
