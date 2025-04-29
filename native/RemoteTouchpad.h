#pragma once
#include <vector>
#include <winsock2.h>
#include <functional>
#include <cstdint>
#include <memory>
#include <string>

#include "LocalServer.h"

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
    std::unique_ptr<LocalServer> server;

    std::vector<std::string> split(const std::string &str, char delimiter);
    bool getScreenSize();
    bool handleInput(std::vector<char> data);

public:
    RemoteTouchPad(uint16_t port);
    bool init();
    static void simulateMouseMovement(float cursorXFraction, float cursorYFraction, int screenWidth, int screenHeight, ClickType clickType);
};
