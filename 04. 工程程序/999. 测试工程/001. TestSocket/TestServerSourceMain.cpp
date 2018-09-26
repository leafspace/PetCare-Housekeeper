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
    
    cout << "SocketServer is listening ..." << endl;
    do {
        char strSendMessage[] = "Server : Hello";
        state = socketServer->acceptLink();
        socketServer->recvMessage();
        cout << socketServer->getMessage() << endl;
        socketServer->setMessage((uint8_t*)strSendMessage, strlen(strSendMessage));
        socketServer->sendMessage();
    } while(false);

    state = socketServer->closeServerSocket();
    delete socketServer;
    return 0;
}
