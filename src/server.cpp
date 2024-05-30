#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <sstream>

#include "common.h"
#include "server.h"

Server* Server::inst = NULL;

Server::Server(int port, int maxConnect) : port(port), maxConnect(maxConnect), currentConnect(0), running(true)
{
    inst = this;

    log = spdlog::basic_logger_mt("basic_logger", "../logs/server");
    spdlog::set_level(spdlog::level::debug);
    log->info("Server instance created on port {}", port);
}

void Server::startServer() 
{
    struct sockaddr_in serverAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    if (serverSocket < 0) 
    {
        log->critical("Error opening socket");
        //handleError("Error opening socket");
    }

    log->info("Socket created successfully");
    //std::cout << "Socket created successfully" << std::endl;

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) 
    {
        log->critical("Error on binding");
        //handleError("Error on binding");
    }

    log->info("Socket bound successfully");

    if (listen(serverSocket, 5) < 0) 
    {
        log->critical("Error on listening");
        //handleError("Error on listening");
    }

    log->info("Server listening on port {}", port);
    //std::cout << "Server listening on port " << port << std::endl;

    processorThread = std::thread(&Server::processCommands, this);

    while (running) 
    {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);

        std::lock_guard<std::mutex> lock(connections_mutex);
        if (currentConnect >= maxConnect)
        {
            log->warn("Max connections limit reached.");
            close(clientSocket);
            continue;
        }

        /*if (clientSocket < 0) 
        {
            if (running) 
            {
                log->error("Error on accept");
                //handleError("Error on accept");
            } 
            else 
            {
                break;
            }
        }*/     

        currentConnect++;

        log->info("Client connected, current connections: {}", currentConnect);
        //std::cout << "Client connected" << std::endl;

        std::thread clientThread(&Server::handleClient, this, clientSocket);
        clientThread.detach();
    }

    close(serverSocket);
    //std::cout << "Server stopped" << std::endl;
}

void Server::stopServer() 
{
    //std::cout << "Stopping server..." << std::endl;
    running = false;
    queue_cq.notify_all();

    if (processorThread.joinable())
    {
        processorThread.join();
    }   

    close(serverSocket);

    log->info("Server stopped");
    //std::cout << "Server stopped" << std::endl;
}

void Server::processCommands() 
{
    while (running) 
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        
        queue_cq.wait(lock, [this] { return !commandQueue.empty() || !running; });

        if (!running && commandQueue.empty())
        {
            break;
        }

        auto clientCommand = commandQueue.front();
        commandQueue.pop();

        lock.unlock();

        int clientSocket = clientCommand.first;
        std::string command = clientCommand.second;

        std::istringstream iss(command);
        std::string cmd;
        std::string key;
        std::string value;

        iss >> cmd >> key >> value;

        std::string response;

        if (cmd == "PUT") 
        {
            //iss >> key >> value;
            
            std::lock_guard<std::mutex> storeLock(store_mutex);
            auto it = store.find(key);

            if (it != store.end())
            {
                response = "OK " + it->second + "\n";
                it->second = value;
                log->debug("Added key {}: {}", key, value);
            }
            else
            {
                store[key] = value;
                response = "OK\n";
                log->debug("Added key {}: {}", key, value);
            }
            
        } 
        else if (cmd == "GET") 
        {
            //iss >> key;

            std::lock_guard<std::mutex> storeLock(store_mutex);
            auto it = store.find(key);

            if (it != store.end()) 
            {
                response = "OK " + it->second + "\n";
                log->debug("Get key {}: {}", key, it->second);
            } 
            else 
            {
                response = "NE\n";
                log->warn("Key {} not found", key);
            }
        } 
        else if (cmd == "DEL") 
        {
            //iss >> key;
            
            std::lock_guard<std::mutex> storeLock(store_mutex);
            auto it = store.find(key);
            
            if (it != store.end())
            {
                response = "OK " + it->second + "\n";
                store.erase(it);
                log->debug("Deleted key {}: {}", key, it->second);
            } 
            else 
            {
                response = "NE\n";
                log->warn("Key {} not found", key);
            }
        } 
        else if (cmd == "COUNT") 
        {
            std::lock_guard<std::mutex> storeLock(store_mutex);
            response = "OK " + std::to_string(store.size()) + "\n";
            log->debug("Counted keys: {}", store.size());
        } 
        else 
        {
            response = "ERR\n";
            log->error("Uknown command: {}", cmd);
        }

        //std::cout << command << std::endl;
        std::cout << response;
    }
}

void Server::handleClient(int clientSocket) 
{
    char buffer[BUFFER_SIZE];

    while (running) 
    {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytesReceived < 0) 
        {
            log->error("Error reading from socket");
            //handleError("Error reading from socket");
        } 
        else if (bytesReceived == 0) 
        {
            log->info("Client disconnected");
            //std::cout << "Client disconnected" << std::endl;
            close(clientSocket);
            break;
        }
        
        std::string command(buffer);

        std::lock_guard<std::mutex> lock(queue_mutex);
        commandQueue.push({clientSocket, command});
        queue_cq.notify_one();
    }

    std::lock_guard<std::mutex> lock(connections_mutex);
    currentConnect--;
    
    log->info("Client disconnect, current connections: {}", currentConnect);
}

void Server::signalHandler(int signal)
{
    if (signal == SIGINT && inst != NULL)
    {
        inst->stopServer();
    }
}

void Server::registerSignal()
{
    std::signal(SIGINT, Server::signalHandler);
}