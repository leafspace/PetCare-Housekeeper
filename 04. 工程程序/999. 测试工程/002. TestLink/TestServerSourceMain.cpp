#include "../../000. PetCarePJ/HouseKeeperRobot/Communication/SocketLink/CommandLink/Server/ServerLink.h"

#include <iostream>
using namespace std;

int main()
{
    bool state = false;
    ServerLink* serverLink = new ServerLink();
    do {
        state = serverLink->linkClient();
        if (state == false) {
            cout << "Link Client failure !" << endl;
            exit(0);
        }
        serverLink->analyzeCommond();
    } while(false);
    
    serverLink->shutdownLink();
    delete serverLink;

    return 0;
}