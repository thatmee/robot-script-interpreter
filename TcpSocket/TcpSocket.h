#pragma once
#include <string>
#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

class TcpSocket
{
public:
    TcpSocket();
    TcpSocket(SOCKET socket);
    ~TcpSocket();
    int connectToHost(std::string ip, unsigned short port);
    int sendMsg(std::string msg);
    std::string recvMsg();

private:
    int readn(char* buf, int size);
    int writen(const char* msg, int size);

private:
    SOCKET m_fd;	// 通信的套接字
};

