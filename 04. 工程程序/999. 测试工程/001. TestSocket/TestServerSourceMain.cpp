#include "../../000. PetCarePJ/HouseKeeperRobot/Communication/SocketLink/BaseSocket/Server/SocketServer.h"

#include <iostream>
using namespace std;

int main()
{
    bool state = false;
    SocketServer* socketServer = new SocketServer();
    state = socketServer->openServerSocket();
    if (state == false) {
        cout << "Can't open server socket !" << endl;
        exit(0);
    }
    
    state = socketServer->listenClient();
    if (state == false) {
        cout << "Can't find clinet socket !" << endl;
        exit(0);
    }

    do {
        state = socketServer->acceptLink();
        socketServer->recvMessage();
        cout << socketServer->getMessage() << endl;
        socketServer->setMessage((uint8_t*)("Server: Hello"), 13);
        socketServer->sendMessage();
    } while(false);

    socketServer->closeServerSocket();
    delete socketServer;
    return 0;
}