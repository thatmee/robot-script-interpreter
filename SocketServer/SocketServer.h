#pragma once
#include <cstdio>
#include <iostream>
#include <cstring>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <lib\magic_enum.hpp>
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")
#include "Interpreter/ScriptServer.h"

using ClientSockID = std::string;
using IdSockMap = std::unordered_map<ClientSockID, SOCKET>;
using IdSockPair = std::pair<ClientSockID, SOCKET>;
using SockHandleMap = std::unordered_map<SOCKET, HANDLE>;
using SockHandlePair = std::pair<SOCKET, HANDLE>;
using SockFlagMap = std::unordered_map<SOCKET, bool>;
using SockFlagPair = std::pair<SOCKET, bool>;

class SocketServer
{
private:
    /************************** 错误处理 ***********************************/

    /// @brief 所有错误状态
    enum class ERR_STA
    {
        WSAStartup_ReturnNoneZero,
        WSADATA_versionWrong,
        socket_INVALID_SOCKET,
        bind_SOCKET_ERROR,
        listen_SOCKET_ERROR,
        accept_SOCKET_ERROR,
        create_thread_ERROR,
        cli_ID_duplicated_ERROR,
        cli_ID_not_exist_ERROR,
        no_socket_ERROR
    };

    /// @brief 错误处理函数
    /// @param err 错误状态
    void error(SocketServer::ERR_STA err);


    /*********************** socket 初始化 **********************************/

    /// @brief 套接字库
    WSADATA wsaData;

    /// @brief 服务器端用来监听的套接字
    SOCKET srvSocket;

    /// @brief 连接的客户端地址族
    SOCKADDR_IN clientAddr;

    /// @brief 服务器地址族
    SOCKADDR_IN srvAddr;

    /// @brief 默认最大连接数为 10
    int connectNumMax = 10;

    /// @brief 初始化 socket 服务器端
    void initSocketServer();


    /******************** socket 读写函数的封装 *******************************/

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


    /*********************** C/S 通信 ***************************************/

    /// @brief 连接到的所有客户端套接字
    IdSockMap cliSockMap;

    /// @brief 针对不同发送消息线程的同步信号量
    SockHandleMap cliSockSyncMap;

    /// @brief 用于安全退出线程的标志
    SockFlagMap killThrd;

    /// @brief 令其能互斥成功正常通信的信号量句柄
    HANDLE bufferMutex;

    /// @brief 提供 sendManager 和 cliSend 线程之间同步的信号量
    HANDLE sendSync;

    /// @brief 用于管理所有发送任务的线程句柄
    //HANDLE sendManagerThread;

    /// @brief 管理整个服务器的关闭
    HANDLE allKill;

    /// @brief 管理所有发送任务的线程
    /// @param IpParameter this 指针
    DWORD static WINAPI ManageSendThread(LPVOID IpParameter);

    /// @brief 针对单个客户端发送消息的线程
    /// @param IpParameter 结构体 Para { p = this; sock = 客户端对应 socket; }
    DWORD static WINAPI CliSendMessageThread(LPVOID IpParameter);

    /// @brief 针对单个客户端接收消息的线程
    /// @param IpParameter 结构体 Para { p = this; sock = 客户端对应 socket; }
    DWORD static WINAPI ReceiveMessageThread(LPVOID IpParameter);


    /*********************** script interpreter ******************************/

    DBG dbg;
    ScriptServer scriptSrv;
    SockHandleMap listenSync;
    SockFlagMap listenValid;

public:
    SocketServer(int connectNumMax_);

    ~SocketServer();

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

/// @brief 用于向线程传递参数的结构体
struct Para {
    SocketServer* p;
    SOCKET sock;
    ClientSockID id;
};

