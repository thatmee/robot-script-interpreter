#pragma once
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <lib\magic_enum.hpp>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")



class SocketClient
{
private:
    /************************** 错误处理 ***********************************/

    /// @brief 所有错误状态
    enum class ERR_STA
    {
        WSAStartup_ReturnNoneZero,
        WSADATA_versionWrong,
        socket_INVALID_SOCKET,
        connect_ERROR
    };

    /// @brief 错误处理函数
    /// @param err 错误状态
    void error(SocketClient::ERR_STA err);


    /*********************** socket 初始化 **********************************/

    /// @brief 套接字库
    WSADATA wsaData;

    /// @brief 客户端的套接字
    SOCKET cliSocket;

    /// @brief 服务器地址族
    SOCKADDR_IN srvAddr;

    /// @brief 初始化客户端 socket
    void initSocketClient();


    /******************** socket 读写函数的封装 *******************************/

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


    /*********************** C/S 通信 ***************************************/

    /// @brief 令其能互斥成功正常通信的信号量句柄
    HANDLE bufferMutex;

    /// @brief 发送消息的线程
    HANDLE sendThread;

    /// @brief 接收消息的线程
    HANDLE receiveThread;

    /// @brief 用于安全退出线程的标志
    bool killThrd;

    /// @brief 当前客户端用于发送消息的线程
    /// @param IpParameter this 指针
    DWORD static WINAPI SendMessageThread(LPVOID IpParameter);

    /// @brief 当前客户端用于接收消息的线程
    /// @param IpParameter this 指针
    DWORD static WINAPI ReceiveMessageThread(LPVOID IpParameter);


public:

    SocketClient();

    ~SocketClient();

    /// @brief 等待线程结束
    void waitThread();

    /// @brief 向对端发送消息
    /// @param msg 要发送的消息字符串
    /// @return -1 表示发送失败，否则为成功发送的字符数
    int Send(std::string msg);

    /// @brief 接收对端消息
    /// @param msg 保存接收到的消息
    /// @return -1 表示接收失败，否则为成功接收的字符数
    int Recv(std::string& msg);
};

