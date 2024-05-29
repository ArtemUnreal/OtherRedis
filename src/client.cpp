#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <csignal>

#include "common.h"

int clientSocket;

void stopClient(int signal) 
{
    std::cout << "Stopping client..." << std::endl;
    close(clientSocket);
    std::cout << "Client Stopped" << std::endl;
    exit(0);
}

void startClient(const std::string &hostname, int port) 
{
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0) 
    {
        handleError("Error opening socket");
    }

    std::cout << "Socket created successfully" << std::endl;

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, hostname.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);

    std::cout << "Connecting to server..." << std::endl;

    while (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) 
    {
        std::cerr << "Error connecting, retrying..." << std::endl;
        sleep(1);
    }

    std::cout << "Connected to server" << std::endl;

    while (true) 
    {
        std::string command;
        std::cout << "> ";
        std::getline(std::cin, command);

        if (command.empty()) 
        {
            continue;
        }

        ssize_t bytesSent = send(clientSocket, command.c_str(), command.length(), 0);

        if (bytesSent < 0) 
        {
            handleError("Error sending command");
        } 
        else 
        {
            std::cout << "Sent " << bytesSent << " bytes" << std::endl;
        }
    }

    close(clientSocket);
}

int main(int argc, char *argv[]) 
{
    if (argc < 3) 
    {
        std::cerr << "Usage: " << argv[0] << " <hostname> <port>" << std::endl;
        return 1;
    }

    std::string hostname = argv[1];
    int port = std::stoi(argv[2]);

    std::signal(SIGINT, stopClient); // Ctrl+C

    startClient(hostname, port);
    
    return 0;
}
