#pragma once

#include <string>
#include <netinet/in.h>
#include <cstdlib>
#include <unistd.h>

const int BUFFER_SIZE = 1024;

void handleError(const std::string& msg)
{
    perror(msg.c_str());
    exit(EXIT_FAILURE);
}