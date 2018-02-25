#include "SocketClient.h"

SocketClient::SocketClient(const char* serverHost = "192.168.1.1")
{
    const uint8_t hostSize = 16;
    this->fileBuffer = new uint8_t[BUFFER_SIZE];
    this->serverHost = new uint8_t[hostSize];
    memset(&(this->fileBuffer), 0, BUFFER_SIZE);
    memset(&(this->serverHost), 0, hostSize);

    this->setServerHost(serverHost);
}

SocketClient::~SocketClient()
{
    delete this->fileBuffer;
    delete this->serverHost;
}

void SocketClient::initBase() 
{
    this->client_socket = 0;
    memset(&(this->client_addr), 0, sizeof(client_addr));
    memset(&(this->server_addr), 0, sizeof(client_addr));
}

void SocketClient::initClient() 
{
    this->client_addr.sin_port = htons(0);
    this->client_addr.sin_family = AF_INET;
    this->client_addr.sin_addr.s_addr = htons(INADDR_ANY);
}

bool SocketClient::bindClient() 
{
    // 创建TCP类型的Socket
    this->client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->client_socket < 0) {
        return false;
    }

    // Socket绑定Socket地址结构
    if (bind(this->client_socket, (struct sockaddr*)&this->client_addr, sizeof(client_addr))) {
        return false;
    }
    return true;
}

void SocketClient::initServer() 
{
    this->server_addr.sin_port = htons(SERVER_PORT);
    this->server_addr.sin_family = AF_INET;
}

void SocketClient::closeLink() 
{
    close(this->client_socket);
}

bool SocketClient::openClientSocket()
{
    this->initBase();
    this->initClient();
    bool state = this->bindClient();
    if (state == false) {
        return false;
    }
    this->initServer();
    return true;
}

bool SocketClient::connectServer() 
{
    if (inet_aton((char*)this->serverHost, &this->server_addr.sin_addr)) {
        return false;
    }

    socklen_t server_addr_length = sizeof(server_addr);
    if (connect(this->client_socket, (struct sockaddr*)&this->server_addr, server_addr_length) < 0) {
        return false;
    }
    return true;
}

int SocketClient::sendMessage() 
{
    return send(this->client_socket, this->fileBuffer, BUFFER_SIZE, 0);
}

int SocketClient::recvMessage() 
{
    int length = recv(this->client_socket, this->fileBuffer, BUFFER_SIZE, 0);
    return length;
}

uint8_t* SocketClient::getMessage()
{
    return this->fileBuffer;
}

bool SocketClient::closeClientSocket()
{
    this->closeLink();
    return true;
}

void SocketClient::setMessage(uint8_t* message, uint32_t messageSize)
{
    memcpy_s(this->fileBuffer, BUFFER_SIZE, message, messageSize);
}

void SocketClient::setServerHost(const char* serverHost)
{
    const uint8_t hostSize = 16;
    memcpy_s(this->serverHost, hostSize, serverHost, strnlen_s(serverHost, hostSize));
}

void setServerPort(const uint32_t serverPort)
{
#ifdef SERVER_PORT
    #undef SERVER_PORT
    #define SERVER_PORT serverPort
#endif
}

void setBufferSize(const uint32_t bufferSize)
{
#ifdef BUFFER_SIZE
    #undef BUFFER_SIZE
    #define BUFFER_SIZE bufferSize
#endif
}