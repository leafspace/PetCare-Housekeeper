#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_HOST     "192.168.1.1"
#define SERVER_PORT     6666
#define BUFFER_SIZE     1024

/* Todo 待确认(UBUNTU下) */
#include <sys/socket.h>
#include <unistd.h>
/* Todo 待确认(UBUNTU下) */

class SocketClient
{
private:
    int client_socket;
    uint8_t *fileBuffer;
    uint8_t *serverHost;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;

    void initBase();
    void initClient();
    bool bindClient();
    void initServer();
public:
    SocketClient() {
        
    }


    bool connectServer(char* serverHost);
    int sendMessage();
    int recvMessage();
    void closeLink();
};
