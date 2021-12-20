#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <Windows.h>
#include "SocketClient.h"

using namespace std;

int main()
{
    SocketClient myClient;
    myClient.waitThread();

    system("pause");
    return 0;
}