# Build the program

$cwd = Get-Location
$gpp_flags = '-O2 -Wall -Wextra -pedantic -std=gnu++17'

cd $cwd

Invoke-Expression "g++ $gpp_flags main.cpp LocalServer.cpp RemoteTouchpad.cpp -o main.exe -lws2_32"