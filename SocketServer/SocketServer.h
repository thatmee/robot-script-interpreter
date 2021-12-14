#pragma once
#include <cstdio>
#include <iostream>
#include <cstring>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <lib\magic_enum.hpp>
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")

class SocketServer
{
private:
    enum class ERR_STA
    {
        WSAStartup_ReturnNoneZero,
        WSADATA_versionWrong,
        socket_INVALID_SOCKET,
        bind_SOCKET_ERROR,
        listen_SOCKET_ERROR,
        accept_SOCKET_ERROR
    };

    int connectNumMax = 10;

    ///// @brief 初始化 socket 以及监听线程
    //void init();

    /// @brief 初始化 socket 服务器端
    void initSocketServer();

    /// @brief 从对端读取指定数量的字符
    /// @param buf 保存读取到的字符
    /// @param size 读取的数量
    /// @return -1 表示读取失败，否则为成功读取的字符数
    int readn(SOCKET srcSocket, char* buf, int size);

    /// @brief 向对端发送指定数量的字符
    /// @param msg 要发送的内容
    /// @param size 要发送的字符数量
    /// @return -1 表示发送失败，否则为成功发送的字符数
    int writen(SOCKET dstSocket, const char* msg, int size);

    /// @brief 错误处理函数
    /// @param err 错误状态
    void error(SocketServer::ERR_STA err);

    DWORD static WINAPI SendMessageThread(LPVOID IpParameter);
    DWORD static WINAPI ReceiveMessageThread(LPVOID IpParameter);
public:

    /// @brief 套接字库
    WSADATA wsaData;

    /// @brief 服务器端用来监听的套接字
    SOCKET srvSocket;

    /*/// @brief 对应客户端连接的套接字
    SOCKET accSocket;*/

    /// @brief 连接到的所有客户端套接字
    std::vector<SOCKET> clientSocketGroup;

    /// @brief 连接的客户端地址族
    SOCKADDR_IN clientAddr;

    /// @brief 服务器地址族
    SOCKADDR_IN srvAddr;

    /// @brief 令其能互斥成功正常通信的信号量句柄
    HANDLE bufferMutex;

    /// @brief 服务器用于发送消息的线程
    HANDLE sendThread;

    SocketServer(int connectNumMax_);

    ~SocketServer();

    /// @brief 等待服务器端的连接
    //void Accept();

    /// @brief 向对端发送消息
    /// @param msg 要发送的消息字符串
    /// @return -1 表示发送失败，否则为成功发送的字符数
    int Send(SOCKET dstSocket, std::string msg);

    /// @brief 接收对端消息
    /// @param msg 保存接收到的消息
    /// @return -1 表示接收失败，否则为成功接收的字符数
    int Recv(SOCKET srcSocket, std::string& msg);

    /// @brief 循环接收客户端连接请求
    void Accept();

    /// @brief 等待线程结束
    void waitThread();
};

struct Para {
    SocketServer* p;
    SOCKET sock;
};

