#include <gtest/gtest.h>
#include <unordered_map>
#include <string>
#include "../src/server.cpp"

std::string funcForTestServer(const std::string& command) 
{
    int clientSocket = 0;

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

    return response;
}

TEST(ServerTest, PutCmd)
{
    EXPECT_EQ(funcForTestServer("PUT name Vasya"), "OK\n");
    EXPECT_EQ(funcForTestServer("PUT name Masha"), "OK Vasya\n");
}

TEST(ServerTes, GetCmd)
{
    funcForTestServer("PUT name Masha");
    EXPECT_EQ(funcForTestServer("GET name"), "OK Masha\n");
    EXPECT_EQ(funcForTestServer("GET age"), "NE\n");
}

TEST(ServerTest, DelCmd)
{
    funcForTestServer("PUT age 20");
    EXPECT_EQ(funcForTestServer("DEL age"), "OK 20\n");
    EXPECT_EQ(funcForTestServer("DEL name"), "NE\n");
}

TEST(ServerTest, CountCmd)
{
    funcForTestServer("PUT name Masha");
    funcForTestServer("PUT age 20");
    EXPECT_EQ(funcForTestServer("COUNT"), "OK 2\n");
    funcForTestServer("DEL name");
    EXPECT_EQ(funcForTestServer("COUNT"), "OK 1\n");
    funcForTestServer("DEL age");
    EXPECT_EQ(funcForTestServer("COUNT"), "OK 0\n");
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

