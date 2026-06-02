#include "../include/server.h"

int main()
{
    OmniGate myserver(8080, {9000, 9001, 9002});
    myserver.start_server();
    return 0;
}