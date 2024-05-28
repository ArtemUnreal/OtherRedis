#include <iostream>
#include <csignal>

#include "server.h"

#ifndef TEST_BUILD
int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);

    Server server(port);

    Server::registerSignal();

    server.startServer();
    
    return 0;
} 
#endif