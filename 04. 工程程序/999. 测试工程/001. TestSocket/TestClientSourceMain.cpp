#include "../../000. PetCarePJ/HouseKeeperRobot/Communication/SocketLink/BaseSocket/Client/SocketClient.h"

#include <iostream>
using namespace std;

int main()
{
    bool state = false;
    SocketClient* socketClient = new SocketClient((uint8_t*)"172.0.0.1", 10000);
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
    do
    {
        socketClient->setMessage((uint8_t*)"Client : Hello", 14);
        socketClient->sendMessage();
        socketClient->recvMessage();
        uint8_t *recvMessage = socketClient->getMessage();
        // Todo recvMessage
        
    } while(false);

    state = socketClient->closeClientSocket();
    delete socketClient;
    return 0;
}