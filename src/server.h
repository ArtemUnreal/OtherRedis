#pragma once

#include <unordered_map>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

class Server 
{
public:
    Server(int port, int maxConnect);
    void startServer();
    void stopServer();

    static void signalHandler(int signal);
    static void registerSignal();
private:
    void processCommands();
    void handleClient(int clientSocket);

    int port;
    int serverSocket;
    int maxConnect;

    std::atomic<int> currentConnect;
    std::mutex connections_mutex;

    std::unordered_map<std::string, std::string> store;
    std::mutex store_mutex;

    std::queue<std::pair<int, std::string>> commandQueue;
    std::mutex queue_mutex;

    std::condition_variable queue_cq;
    std::atomic<bool> running;

    std::thread processorThread;

    static Server* inst;

    std::shared_ptr<spdlog::logger> log;
};