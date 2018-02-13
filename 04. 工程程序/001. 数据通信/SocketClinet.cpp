#include "SocketClient.h"

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

bool SocketClient::connectServer(char* serverHost) 
{
    if (inet_aton(SERVER_HOST, &this->server_addr.sin_addr)) {
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
}

void SocketClient::closeLink() 
{
    close(this->client_socket);
}