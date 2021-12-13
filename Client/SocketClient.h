#pragma once
#include <iostream>
#include <string>
#include <lib\magic_enum.hpp>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

class SocketClient
{
private:
    enum class ERR_STA
    {
        WSAStartup_ReturnNoneZero,
        WSADATA_versionWrong,
        socket_INVALID_SOCKET,
        connect_ERROR
    };

    int connectNumMax = 10;

    /// @brief 错误处理函数
    /// @param err 错误状态
    void error(SocketClient::ERR_STA err);

    /// @brief 初始化客户端 socket
    void initSocketClient();

    /// @brief 从对端读取指定数量的字符
    /// @param buf 保存读取到的字符
    /// @param size 读取的数量
    /// @return -1 表示读取失败，否则为成功读取的字符数
    int readn(char* buf, int size);

    /// @brief 向对端发送指定数量的字符
    /// @param msg 要发送的内容
    /// @param size 要发送的字符数量
    /// @return -1 表示发送失败，否则为成功发送的字符数
    int writen(const char* msg, int size);

public:

    /// @brief 套接字库
    WSADATA wsaData;

    /// @brief 客户端的套接字
    SOCKET cliSocket;

    /// @brief 服务器地址族
    SOCKADDR_IN srvAddr;

    SocketClient();
    ~SocketClient();

    /// @brief 向对端发送消息
    /// @param msg 要发送的消息字符串
    /// @return -1 表示发送失败，否则为成功发送的字符数
    int Send(std::string msg);

    /// @brief 接收对端消息
    /// @param msg 保存接收到的消息
    /// @return -1 表示接收失败，否则为成功接收的字符数
    int Recv(std::string& msg);
};

