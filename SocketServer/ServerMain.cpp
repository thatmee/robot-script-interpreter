#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <vector>
#include <iterator>
#include <algorithm>
#include <Winsock2.h>
#include <Windows.h>
#include "SocketServer.h"

using namespace std;

const int MAX_USERS = 20;

int main()
{
    SocketServer myServer(MAX_USERS);
    myServer.Accept();
    myServer.waitThread();


    system("pause");
    return 0;
}