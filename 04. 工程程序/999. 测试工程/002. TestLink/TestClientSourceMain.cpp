#include "../../000. PetCarePJ/HouseKeeperRobot/Communication/SocketLink/CommandLink/Client/ClientLink.h"

#include <iostream>
using namespace std;

int main()
{
    bool state = false;
    ClientLink* clientLink = new ClientLink("127.0.0.1", 6666);
    do {
        state = clientLink->linkServer();
        if (state == false) {
            cout << "Link Server failure !" << endl;
            exit(0);
        }

        clientLink->sendFile("testfile.exe");
    } while(false);

    clientLink->shutdownLink();
    delete clientLink;

    return 0;
}