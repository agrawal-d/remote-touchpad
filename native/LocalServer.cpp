#include "LocalServer.h"
#include <iostream>
#include <winsock2.h>
#include <windows.h>

using namespace std;

LocalServer::LocalServer(uint16_t port, std::function<void(vector<char>)> callback)
    : port(port), callback(callback), sock(INVALID_SOCKET)
{
}

bool LocalServer::initializeWinsock()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup failed" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

bool LocalServer::createSocket()
{
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        cout << "Socket creation failed" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

bool LocalServer::bindSocket()
{
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cout << "Bind failed" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

bool LocalServer::printDeviceIp()
{
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
    {
        cout << "gethostname failed" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
        return false;
    }

    struct hostent *host = gethostbyname(hostname);
    if (host == NULL)
    {
        cout << "gethostbyname failed" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
        return false;
    }

    char *ip = inet_ntoa(*(struct in_addr *)*host->h_addr_list);
    if (ip == NULL)
    {
        cout << "inet_ntoa failed" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
        return false;
    }

    cout << "Device IP address: " << ip << endl;
    return true;
}

bool LocalServer::startListening()
{
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
    {
        cout << "Listen failed" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
        return false;
    }

    cout << "Server started on port " << port << endl;
    cout << "Waiting for connections..." << endl;

    if (!printDeviceIp())
    {
        cout << "Failed to print device IP address" << endl;
        return false;
    }

    return true;
}

void LocalServer::handleClient(SOCKET clientSock)
{
    char buffer[1024];
    int bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);
    if (bytesReceived == SOCKET_ERROR)
    {
        cout << "Receive failed" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
    }
    else
    {
        cout << "Received " << bytesReceived << " bytes" << endl;
        vector<char> data(buffer, buffer + bytesReceived);
        callback(data);
    }
    closesocket(clientSock);
}

void LocalServer::acceptConnections()
{
    while (true)
    {
        SOCKET clientSock = accept(sock, NULL, NULL);
        if (clientSock == INVALID_SOCKET)
        {
            cout << "Accept failed" << endl;
            cout << "Error: " << WSAGetLastError() << endl;
            continue;
        }
        cout << "Client connected" << endl;
        handleClient(clientSock);
    }
}

bool LocalServer::init()
{
    if (!initializeWinsock())
        return false;

    if (!createSocket())
        return false;

    if (!bindSocket())
        return false;

    if (!startListening())
        return false;

    acceptConnections();
    return true;
}

LocalServer::~LocalServer()
{
    if (sock != INVALID_SOCKET)
    {
        closesocket(sock);
        WSACleanup();
    }
}
