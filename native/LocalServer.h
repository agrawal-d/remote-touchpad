#pragma once
#include <vector>
#include <winsock2.h>
#include <functional>
#include <cstdint>

#include "Logger.h"

class LocalServer
{
private:
    uint16_t port;
    std::function<void(std::vector<char>)> callback;
    SOCKET sock;

    bool initializeWinsock();
    bool createSocket();
    bool bindSocket();
    bool printDeviceIp();
    bool startListening();
    void handleClient(SOCKET clientSock);
    void acceptConnections();

public:
    LocalServer(uint16_t port, std::function<void(std::vector<char>)> callback);
    bool init();
    ~LocalServer();
};
