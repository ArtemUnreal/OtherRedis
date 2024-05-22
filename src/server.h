#pragma once

void processCommands();
void handleClient(int clientSocket);
void startServer(int port);
void stopServer(int signal);
