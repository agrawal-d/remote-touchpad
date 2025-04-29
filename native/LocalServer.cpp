#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#include "LocalServer.h"

const string LocalServer::DATA_KEY = "data=";
const string LocalServer::HTTP_KEY = " HTTP";
const string LocalServer::HTTP_RESPONSE = "HTTP/1.1 200 OK\r\nContent-Length: 7\r\nAccess-Control-Allow-Origin: *\r\n\r\nSuccess";
const string LocalServer::TEST_CONNECTION = "test_connection";

static bool GetIPAddress(string &outAddress)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return false;
    }

    ULONG bufferSize = 15000;
    IP_ADAPTER_ADDRESSES *adapterAddresses = (IP_ADAPTER_ADDRESSES *)malloc(bufferSize);
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG family = AF_INET;

    DWORD dwRetVal = GetAdaptersAddresses(family, flags, NULL, adapterAddresses, &bufferSize);

    if (dwRetVal == ERROR_BUFFER_OVERFLOW)
    {
        free(adapterAddresses);
        adapterAddresses = (IP_ADAPTER_ADDRESSES *)malloc(bufferSize);
        dwRetVal = GetAdaptersAddresses(family, flags, NULL, adapterAddresses, &bufferSize);
    }

    bool success = false;

    if (dwRetVal == NO_ERROR)
    {
        for (IP_ADAPTER_ADDRESSES *aa = adapterAddresses; aa != NULL; aa = aa->Next)
        {
            // Skip adapters that are down or don't support IPv4
            if (aa->OperStatus != IfOperStatusUp || !(aa->Flags & IP_ADAPTER_IPV4_ENABLED))
                continue;

            for (IP_ADAPTER_UNICAST_ADDRESS *ua = aa->FirstUnicastAddress; ua != NULL; ua = ua->Next)
            {
                if (ua->Address.lpSockaddr->sa_family == AF_INET)
                {
                    SOCKADDR_IN *sa_in = (SOCKADDR_IN *)ua->Address.lpSockaddr;
                    char ipStr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(sa_in->sin_addr), ipStr, sizeof(ipStr));

                    // Ensure the IP is not the default gateway (e.g., 192.168.0.1)
                    if (strcmp(ipStr, "127.0.0.1") != 0 && strcmp(ipStr, "192.168.0.1") != 0) // Skip localhost
                    {
                        outAddress = ipStr;
                        success = true;
                        break;
                    }
                }
            }
            if (success)
            {
                break; // Exit early after finding one address
            }
        }
    }

    if (adapterAddresses)
    {
        free(adapterAddresses);
    }

    WSACleanup();

    return success;
}

LocalServer::LocalServer(uint16_t port, function<void(string)> callback)
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
    string ipAddress;
    if (!GetIPAddress(ipAddress))
    {
        cout << "Run ipconfig in cmd to get your Local IPV4 address. Use this in the web-application on your phone." << endl;
    }
    else
    {
        cout << "IP Address: " << ipAddress << endl;
    }

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

    cout << "Remote touchpad server started" << endl;

    if (!printDeviceIp())
    {
        cout << "Failed to print device IP address" << endl;
        return false;
    }
    cout << "Port: " << port << endl;

    return true;
}

void LocalServer::handleClient(SOCKET clientSock)
{
    char buffer[4096]; // 4KiB buffer
    string input;

    while (true)
    {
        int bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR)
        {
            cout << "Receive failed" << endl;
            cout << "Error: " << WSAGetLastError() << endl;
            break;
        }
        else if (bytesReceived == 0)
        {
            cout << "Client disconnected" << endl;
            break;
        }
        else
        {
            input.insert(input.end(), buffer, buffer + bytesReceived);
            if (bytesReceived >= 4 && buffer[bytesReceived - 4] == '\r' && buffer[bytesReceived - 3] == '\n' &&
                buffer[bytesReceived - 2] == '\r' && buffer[bytesReceived - 1] == '\n')
            {
                break;
            }
        }
    }

    if (input.find(TEST_CONNECTION) != string::npos)
    {
        auto client_ip = inet_ntoa(((sockaddr_in *)&clientSock)->sin_addr);
        cout << "New client connected with IP: " << client_ip << endl;
        send(clientSock, HTTP_RESPONSE.data(), HTTP_RESPONSE.size(), 0);
        closesocket(clientSock);
        return;
    }

    size_t dataPos = input.find(DATA_KEY);
    size_t endPos = input.find(HTTP_KEY, dataPos);

    if (dataPos == string::npos || endPos == string::npos)
    {
        cout << "Invalid input format" << endl;
    }
    else
    {
        string contents = input.substr(dataPos + 5, endPos - dataPos - 5);
        callback(contents);
    }

    // Send a success empty HTTP response
    send(clientSock, HTTP_RESPONSE.data(), HTTP_RESPONSE.size(), 0);
    closesocket(clientSock);
}

void LocalServer::acceptConnections()
{
    while (running)
    {
        SOCKET clientSock = accept(sock, NULL, NULL);
        if (clientSock == INVALID_SOCKET)
        {
            if (running) // Check if still running to avoid printing errors during shutdown
            {
                cout << "Accept failed" << endl;
                cout << "Error: " << WSAGetLastError() << endl;
            }
            continue;
        }
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

    running = true;
    connectionThread = thread(&LocalServer::acceptConnections, this);
    return true;
}

void LocalServer::forever()
{
    if (connectionThread.joinable())
    {
        connectionThread.join();
    }
}

LocalServer::~LocalServer()
{
    WSACleanup();
}
