#include <iostream>
#include <csignal>

#include "server.h"

#ifndef TEST_BUILD
int main(int argc, char *argv[]) 
{
    if (argc < 3) 
    {
        std::cerr << "Usage: " << argv[0] << " <port> <max_connections>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);
    int maxConnect = std::stoi(argv[2]);

    Server server(port, maxConnect);

    Server::registerSignal();

    server.startServer();
    
    return 0;
} 
#endif