#pragma once
#include <vector>
#include <winsock2.h>
#include <functional>
#include <cstdint>
#include <atomic>
#include <thread>
#include <iostream>

#include "Logger.h"

using namespace std;

class LocalServer
{
private:
    uint16_t port;
    function<void(string)> callback;
    SOCKET sock;
    thread connectionThread;
    atomic<bool> running;
    static const string DATA_KEY;
    static const string HTTP_KEY;
    static const string HTTP_RESPONSE;

    bool initializeWinsock();
    bool createSocket();
    bool bindSocket();
    bool printDeviceIp();
    bool startListening();
    void handleClient(SOCKET clientSock);
    void acceptConnections();

public:
    LocalServer(uint16_t port, function<void(string)> callback);
    bool init();
    void forever();
    ~LocalServer();
};
