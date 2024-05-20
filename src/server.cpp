#include <iostream>
#include <thread>
#include <unordered_map>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <csignal>
#include <sstream>

#include "common.h"

std::unordered_map<std::string, std::string> store;
std::mutex store_mutex;

std::queue<std::pair<int, std::string>> commandQueue;
std::mutex queue_mutex;

std::condition_variable queue_cq;

std::atomic<bool> running(true);

void processCommands() 
{
    while (running) 
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cq.wait(lock, [] { return !commandQueue.empty() || !running; });

        if (!running && commandQueue.empty()) break;

        auto clientCommand = commandQueue.front();
        commandQueue.pop();
        lock.unlock();

        int clientSocket = clientCommand.first;
        std::string command = clientCommand.second;

        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;

        std::string key;
        std::string value;
        std::string response;

        if (cmd == "PUT") 
        {
            iss >> key >> value;
            
            std::lock_guard<std::mutex> storeLock(store_mutex);
            auto it = store.find(key);

            if (it != store.end())
            {
                response = "OK " + it->second + "\n";
                it->second = value;
            }
            else
            {
                store[key] = value;
                response = "OK\n";
            }
            
        } 
        else if (cmd == "GET") 
        {
            iss >> key;

            std::lock_guard<std::mutex> storeLock(store_mutex);
            auto it = store.find(key);

            if (it != store.end()) 
            {
                response = "OK " + it->second + "\n";
            } 
            else 
            {
                response = "NE\n";
            }
        } 
        else if (cmd == "DEL") 
        {
            iss >> key;
            
            std::lock_guard<std::mutex> storeLock(store_mutex);
            auto it = store.find(key);
            
            if (it != store.end())
            {
                response = "OK " + it->second + "\n";
                store.erase(it);
            } 
            else 
            {
                response = "NE\n";
            }
        } 
        else if (cmd == "COUNT") 
        {
            std::lock_guard<std::mutex> storeLock(store_mutex);
            response = "OK " + std::to_string(store.size()) + "\n";
        } 
        else 
        {
            response = "ERR Unknown command";
        }

        std::cout << command << std::endl;
        std::cout << response << std::endl;
    }
}

void handleClient(int clientSocket) 
{
    char buffer[BUFFER_SIZE];

    while (running) 
    {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytesReceived < 0) 
        {
            handleError("Error reading from socket");
        } 
        else if (bytesReceived == 0) 
        {
            std::cout << "Client disconnected" << std::endl;
            close(clientSocket);
            break;
        }
        
        std::string command(buffer);

        std::lock_guard<std::mutex> lock(queue_mutex);
        commandQueue.push({clientSocket, command});
        queue_cq.notify_one();
    }
}

void startServer(int port) 
{
    int serverSocket;
    struct sockaddr_in serverAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    if (serverSocket < 0) 
    {
        handleError("Error opening socket");
    }

    std::cout << "Socket created successfully" << std::endl;

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) 
    {
        handleError("Error on binding");
    }

    std::cout << "Socket bound successfully" << std::endl;

    if (listen(serverSocket, 5) < 0) 
    {
        handleError("Error on listening");
    }

    std::cout << "Server listening on port " << port << std::endl;

    std::thread processorThread(processCommands);
    processorThread.detach();

    while (running) 
    {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);

        if (clientSocket < 0) 
        {
            if (running) 
            {
                handleError("Error on accept");
            } 
            else 
            {
                break;
            }
        }

        std::cout << "Client connected" << std::endl;

        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }

    close(serverSocket);
    std::cout << "Server stopped" << std::endl;
}

void stopServer(int signal) 
{
    std::cout << "Stopping server..." << std::endl;
    running = false;
    queue_cq.notify_all();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    exit(0);
}

 int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);

    std::signal(SIGINT, stopServer); 

    startServer(port);
    
    return 0;
} 
