#include <vector>
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <functional>
#include <memory>

#include "RemoteTouchPad.h"
#include "Logger.h"
#include "LocalServer.h"

using namespace std;

RemoteTouchPad::RemoteTouchPad(uint16_t port)
{
    server = make_unique<LocalServer>(port, [this](vector<char> data)
                                      { handleInput(data); });
}

bool RemoteTouchPad::init()
{
    if (!server->init())
    {
        return false;
    }

    if (!getScreenSize())
    {
        return false;
    }

    return true;
}

vector<string> RemoteTouchPad::split(const string &str, char delimiter)
{
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

bool RemoteTouchPad::getScreenSize()
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    screenWidth = desktop.right;
    screenHeight = desktop.bottom;

    if (screenWidth <= 0 || screenHeight <= 0)
    {
        cout << "Failed to get screen size" << endl;
        return false;
    }

    cout << "Screen Size: " << screenWidth << "x" << screenHeight << endl;

    return true;
}

bool RemoteTouchPad::handleInput(vector<char> data)
{
    // Input is a string CSV of x,y,clickType (clickType is an int)
    string input(data.begin(), data.end());
    auto sections = split(input, ',');

    if (sections.size() != 3)
    {
        cout << "Invalid input format. Expected 3 values but got " << sections.size() << endl;
        return false;
    }

    try
    {
        cursorXFraction = stof(sections[0]);
        cursorYFraction = stof(sections[1]);
        int clickTypeInt = stoi(sections[2]);

        if (clickTypeInt < 0 || clickTypeInt > 3)
        {
            cout << "Invalid click type value: " << clickTypeInt << endl;
            return false;
        }

        clickType = static_cast<ClickType>(clickTypeInt);

        LOG_VERBOSE &&cout << "Cursor Position: (" << cursorXFraction << ", " << cursorYFraction << ")" << endl;
        LOG_VERBOSE &&cout << "Click Type: " << clickTypeInt << endl;

        simulateMouseMovement(cursorXFraction, cursorYFraction, screenWidth, screenHeight, clickType);
    }
    catch (const invalid_argument &e)
    {
        cout << "Invalid input value: " << e.what() << endl;
        return false;
    }

    return true;
}

void RemoteTouchPad::simulateMouseMovement(float cursorXFraction, float cursorYFraction, int screenWidth, int screenHeight, ClickType clickType)
{
    // Calculate x and y
    int x = static_cast<int>(cursorXFraction * screenWidth);
    int y = static_cast<int>(cursorYFraction * screenHeight);

    // Set mouse position
    SetCursorPos(x, y);

    switch (clickType)
    {
    case ClickType::Left:
        mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        break;
    case ClickType::Right:
        mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
        break;
    case ClickType::Middle:
        mouse_event(MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
        break;
    case ClickType::None:
    default:
        break;
    }
}
