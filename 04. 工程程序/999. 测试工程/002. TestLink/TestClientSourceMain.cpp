#include "../../000. PetCarePJ/HouseKeeperRobot/Communication/SocketLink/CommandLink/Client/ClientLink.h"

#include <iostream>
using namespace std;

int main()
{
    bool state = false;
    ClientLink* clientLink = new ClientLink();
    do {
        state = clientLink->linkServer();
        if (state == false) {
            cout << "Link Server failure !" << endl;
        }

        clientLink->sendFile("testfile.exe");
    } while(false);

    clientLink->shutdownLink();
    delete clientLink;

    return 0;
}