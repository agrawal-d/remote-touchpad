#include <vector>
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <functional>
#include <memory>

#include "RemoteTouchPad.h"

using namespace std;

int main()
{
    RemoteTouchPad touchPad(8080);
    if (!touchPad.init())
    {
        cout << "Failed to initialize RemoteTouchPad" << endl;
        return 1;
    }

    touchPad.forever();

    return 0;
}