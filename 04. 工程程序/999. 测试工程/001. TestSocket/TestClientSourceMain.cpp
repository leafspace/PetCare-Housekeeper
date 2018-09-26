#include "../../000. PetCarePJ/HouseKeeperRobot/Communication/SocketLink/BaseSocket/Client/SocketClient.h"

#include <iostream>
using namespace std;

int main()
{
    bool state = false;
    SocketClient* socketClient = new SocketClient((uint8_t*)"127.0.0.1", 6666);
    state = socketClient->openClientSocket();
    if (state == false) {
        cout << "Can't open client socket !" << endl;
        exit(0);
    }

    state = socketClient->connectServer();
    if (state == false) {
        cout << "Can't connect client socket !" << endl;
        exit(0);
    }

    cout << "Connect Success !" << endl;
    do {
        char strSendMessage[] = "Client : Hello";
        socketClient->setMessage((uint8_t*)strSendMessage, strlen(strSendMessage));
        socketClient->sendMessage();
        socketClient->recvMessage();
        cout << socketClient->getMessage() << endl;
    } while(false);

    state = socketClient->closeClientSocket();
    delete socketClient;
    return 0;
}