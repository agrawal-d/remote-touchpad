#pragma once
#include <vector>
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <functional>
#include <memory>

#include "Logger.h"
#include "LocalServer.h"

using namespace std;

enum class ClickType
{
    None = 0,
    Left = 1,
    Right = 2,
    Middle = 3
};

class RemoteTouchPad
{
private:
    int screenWidth;
    int screenHeight;
    float cursorXFraction;
    float cursorYFraction;
    ClickType clickType;
    unique_ptr<LocalServer> server;

    vector<string> split(const string &str, char delimiter);
    bool getScreenSize();
    bool handleInput(string data);

public:
    RemoteTouchPad(uint16_t port);
    bool init();
    void forever();
    static void simulateMouseMovement(float cursorXFraction, float cursorYFraction, int screenWidth, int screenHeight, ClickType clickType);
};
