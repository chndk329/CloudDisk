#include "../include/CloudDiskServer.h"
#include <iostream>
using namespace std;

int main(void)
{
    CloudDiskServer server;
    server.start(8888);
    return 0;
}

