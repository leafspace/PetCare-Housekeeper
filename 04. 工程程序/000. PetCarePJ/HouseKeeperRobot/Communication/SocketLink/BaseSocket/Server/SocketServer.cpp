#include "SocketServer.h"

SocketServer::SocketServer()
{
    this->fileBuffer = new uint8_t[BUFFER_SIZE];
    memset(this->fileBuffer, 0, BUFFER_SIZE);
}

SocketServer::~SocketServer()
{
    delete this->fileBuffer;
}

void SocketServer::initBase() 
{
    bzero(&(this->client_addr), sizeof(this->client_addr));
    bzero(&(this->server_addr), sizeof(this->server_addr));
    memset(&(this->client_addr), 0, sizeof(this->client_addr));
    memset(&(this->server_addr), 0, sizeof(this->client_addr));
}

void SocketServer::initServer()
{
    this->server_addr.sin_port = htons(SERVER_PORT);
    this->server_addr.sin_family = AF_INET;
    this->server_addr.sin_addr.s_addr = htons(INADDR_ANY);
}

bool SocketServer::bindServer()
{
    this->server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (this->server_socket < 0) {
        return false;
    }

    if (bind(this->server_socket, (struct sockaddr*)&this->server_addr, 
        sizeof(this->server_addr))) {
        return false;
    }
    return true;
}

void SocketServer::closeLink()
{
    close(this->link_socket);
    close(this->server_socket);
}

bool SocketServer::openServerSocket()
{
    bool state = false;

    this->initBase();
    this->initServer();
    state = this->bindServer();
    if (state == false) {
        return false;
    }
    state = this->listenClient();
    if (state == false) {
        return false;
    }
    return true;
}

bool SocketServer::listenClient()
{
    if (listen(server_socket, LISTEN_QUEU)) {
        return false;
    }
    return true;
}

bool SocketServer::acceptLink()
{
    socklen_t length = sizeof(this->client_addr);
    this->link_socket = accept(this->server_socket, (struct sockaddr*)&this->client_addr, &length);
    if (this->link_socket < 0) {
        return false;
    }
    return true;
}

int SocketServer::sendMessage()
{
    return send(this->link_socket, this->fileBuffer, this->messageSize, 0);
}

int SocketServer::recvMessage()
{
    return recv(this->link_socket, this->fileBuffer, BUFFER_SIZE, 0);
}

uint8_t* SocketServer::getMessage()
{
    return this->fileBuffer;
}

bool SocketServer::closeServerSocket()
{
    this->closeLink();
    return true;
}

void SocketServer::setMessage(const uint8_t* message, uint32_t messageSize)
{
    this->messageSize = messageSize;
    memcpy(this->fileBuffer, message, this->messageSize);
}
