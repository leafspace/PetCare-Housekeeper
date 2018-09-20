#include "SocketClient.h"

SocketClient::SocketClient(const string serverHost = "127.0.0.1", const uint32_t serverPort = 6666)
{
    cout << "Test !" << endl;
    this->fileBuffer = new uint8_t[BUFFER_SIZE];
    assert(this->fileBuffer != NULL);
    memset(&(this->fileBuffer), 0, BUFFER_SIZE);
    this->setServerHost(serverHost);
    this->setServerPort(serverPort);
}

SocketClient::~SocketClient()
{
    delete this->fileBuffer;
}

void SocketClient::initBase() 
{
    this->client_socket = 0;
    memset(&(this->client_addr), 0, sizeof(this->client_addr));
    memset(&(this->server_addr), 0, sizeof(this->client_addr));
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
    if (bind(this->client_socket, (struct sockaddr*)&this->client_addr, 
        sizeof(this->client_addr))) {
        return false;
    }
    return true;
}

void SocketClient::initServer() 
{
    this->server_addr.sin_port = htons(this->serverPort);
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
    printf("%s", this->serverHost);
    if (inet_aton((char*)this->serverHost.data(), &this->server_addr.sin_addr)) {
        return false;
    }

    socklen_t server_addr_length = sizeof(server_addr);
    if (connect(this->client_socket, (struct sockaddr*)&this->server_addr, 
        server_addr_length) < 0) {
        return false;
    }
    return true;
}

int SocketClient::sendMessage() 
{
    return send(this->client_socket, this->fileBuffer, this->messageSize, 0);
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

void SocketClient::setMessage(const uint8_t* message, uint32_t messageSize)
{
    this->messageSize = messageSize;
    memcpy(this->fileBuffer, message, messageSize);
}

void SocketClient::setServerHost(const string serverHost)
{
    this->serverHost = serverHost;
}

void SocketClient::setServerPort(const uint32_t serverPort)
{
    this->serverPort = serverPort;
}
