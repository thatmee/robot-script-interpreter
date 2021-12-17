#include "SocketServer.h"

SocketServer::SocketServer(int connectNumMax_ = 10)
    : connectNumMax(connectNumMax_)
{
    initSocketServer();

    bufferMutex = CreateSemaphore(NULL, 1, 1, NULL);
    sendSync = CreateSemaphore(NULL, 1, 1, NULL);
    sendManagerThread = CreateThread(NULL, 0, ManageSendThread, this, 0, NULL);
}

SocketServer::~SocketServer()
{
    // 关闭连接到的客户 Socket
    for (IdSockMap::iterator iter = cliSockMap.begin(); iter != cliSockMap.end(); iter++)
    {
        if (iter->second)
            closesocket(iter->second);
    }

    // 关闭服务器端监听 socket
    if (srvSocket)
        closesocket(srvSocket);

    // 释放所有句柄
    CloseHandle(bufferMutex);
    CloseHandle(sendSync);
    CloseHandle(sendManagerThread);

}

void SocketServer::error(SocketServer::ERR_STA err)
{

    std::cout << "error: " << magic_enum::enum_name(err) << std::endl;
}

void SocketServer::initSocketServer()
{
    int iRet = 0;

    std::cout << "====================== Server ==========================" << std::endl;

    // 加载 2.2 版本的套接字库
    iRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iRet != 0)
    {
        error(SocketServer::ERR_STA::WSAStartup_ReturnNoneZero);
        return;
    }
    if (2 != LOBYTE(wsaData.wVersion) || 2 != HIBYTE(wsaData.wVersion))
    {
        WSACleanup();
        error(SocketServer::ERR_STA::WSADATA_versionWrong);
        return;
    }
    std::cout << "done! load the socket data." << std::endl;

    // 创建流式套接字
    srvSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (srvSocket == INVALID_SOCKET)
    {
        error(SocketServer::ERR_STA::socket_INVALID_SOCKET);
        return;
    }
    std::cout << "done! create srvSocket." << std::endl;

    // 初始化服务器地址族变量
    srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(6000);
    std::cout << "done! init server address." << std::endl;

    // 绑定
    iRet = bind(srvSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
    if (iRet == SOCKET_ERROR)
    {
        error(SocketServer::ERR_STA::bind_SOCKET_ERROR);
        return;
    }
    std::cout << "done! bind." << std::endl;


    //监听
    iRet = listen(srvSocket, connectNumMax);
    if (iRet == SOCKET_ERROR)
    {
        error(SocketServer::ERR_STA::listen_SOCKET_ERROR);
        return;
    }
    std::cout << "done! listen." << std::endl;
    std::cout << "Server ready!" << std::endl;
    std::cout << std::endl << "===================== by nanyf ==========================" << std::endl << std::endl;
}


int SocketServer::Send(SOCKET dstSocket, std::string msg)
{
    // 申请内存空间: 数据长度 + 包头4字节(存储数据长度)
    char* sendPackage = new char[msg.size() + 4];
    int bigLen = htonl(msg.size());
    memcpy(sendPackage, &bigLen, 4);
    memcpy(sendPackage + 4, msg.data(), msg.size());

    // 发送数据
    int ret = writen(dstSocket, sendPackage, msg.size() + 4);
    delete[] sendPackage;
    return ret;
}

int SocketServer::Recv(SOCKET srcSocket, std::string& msg)
{
    // 读数据头
    int len = 0;
    int ret = readn(srcSocket, (char*)&len, 4);
    if (ret == -1)
    {
        // 没有读到数据头，对端异常
        msg = "";
        return -1;
    }
    len = ntohl(len);

    // 根据读出的长度分配内存
    char* buf = new char[len + 1];
    ret = readn(srcSocket, buf, len);
    if (ret != len)
    {
        msg = "";
        delete[] buf;
        return -1;
    }

    // 根据读出的长度设置字符串结尾
    buf[len] = '\0';
    msg = std::string(buf);
    delete[] buf;
    return len;
}

int SocketServer::readn(SOCKET srcSocket, char* buf, int size)
{
    int nread = 0;
    int left = size;
    char* p = buf;

    while (left > 0)
    {
        if ((nread = recv(srcSocket, p, left, 0)) > 0)
        {
            p += nread;
            left -= nread;
        }
        else if (nread == -1)
            return -1;
    }
    return size;
}

int SocketServer::writen(SOCKET dstSocket, const char* msg, int size)
{
    int left = size;
    int nwrite = 0;
    const char* p = msg;

    while (left > 0)
    {
        if ((nwrite = send(dstSocket, msg, left, 0)) > 0)
        {
            p += nwrite;
            left -= nwrite;
        }
        else if (nwrite == -1)
            return -1;
    }
    return size;
}

DWORD WINAPI SocketServer::ManageSendThread(LPVOID IpParameter)
{
    // 传入了类的 this 指针
    SocketServer* p = (SocketServer*)IpParameter;

    while (1)
    {
        // 上一条消息输入、发送完毕后才处理下一个发送任务
        WaitForSingleObject(p->sendSync, INFINITE);

        std::cout << "请输入目的客户端 ID: ";
        std::string dstID;
        std::getline(std::cin, dstID);

        // wait (bufferMutex)
        WaitForSingleObject(p->bufferMutex, INFINITE);

        if (p->cliSockMap.find(dstID) == p->cliSockMap.end())
        {
            // 目的 ID 不存在，错误处理并释放资源
            p->error(SocketServer::ERR_STA::cli_ID_not_exist_ERROR);
            ReleaseSemaphore(p->bufferMutex, 1, NULL);
            ReleaseSemaphore(p->sendSync, 1, NULL);
            continue;
        }

        // 根据目的客户端的 ID 得到目的客户端的 socket
        SOCKET dstSock = p->cliSockMap[dstID];
        std::cout << "dstSock: " << dstSock << std::endl;

        if (p->cliSockSyncMap.find(dstSock) != p->cliSockSyncMap.end())
        {
            // 如果客户端 socket 的同步信号量已经创建
            // 将信号量置有效，将针对目的客户端发送消息的线程置为非阻塞状态
            ReleaseSemaphore(p->bufferMutex, 1, NULL);
            // signal (bufferMutex)
            ReleaseSemaphore(p->cliSockSyncMap[dstSock], 1, NULL);
        }
        else
        {
            // 如果客户端 socket 的同步信号量还没有创建，进入错误处理模块，跳过本次发送
            p->error(SocketServer::ERR_STA::no_socket_ERROR);
            ReleaseSemaphore(p->sendSync, 1, NULL);
            ReleaseSemaphore(p->bufferMutex, 1, NULL);
        }
    }
}

DWORD WINAPI SocketServer::CliSendMessageThread(LPVOID IpParameter)
{
    // 将传入参数转为原来的结构体类型
    Para* para = (Para*)IpParameter;

    while (1)
    {
        // kill 标志为真，则直接退出循环，结束线程
        if (para->p->killThrd[para->sock])
            break;

        // 所有专用的发送消息线程默认都处于阻塞状态，由 manageSend 线程来控制其变为非阻塞状态
        if (para->p->cliSockSyncMap.find(para->sock) != para->p->cliSockSyncMap.end())
            WaitForSingleObject(para->p->cliSockSyncMap[para->sock], INFINITE);
        else
            continue;

        std::string msg;
        std::getline(std::cin, msg);

        // wait (bufferMutex)
        WaitForSingleObject(para->p->bufferMutex, INFINITE);

        // 向指定的客户端发送消息
        int ret = para->p->Send(para->sock, msg);
        if (ret < 0)
            std::cout << "send error;" << std::endl;
        else
            std::cout << "I say: " << msg << std::endl;

        // signal (bufferMutex)
        ReleaseSemaphore(para->p->bufferMutex, 1, NULL);

        // 当前发送任务完成，允许 manageSend 线程调度下一次发送任务
        ReleaseSemaphore(para->p->sendSync, 1, NULL);
    }
    return 0;
}


DWORD WINAPI SocketServer::ReceiveMessageThread(LPVOID IpParameter)
{
    // 将传入参数转为原来的结构体类型
    Para* para = (Para*)IpParameter;

    while (1)
    {
        std::string recvMsg;
        int ret = para->p->Recv(para->sock, recvMsg);

        // wait （bufferMutex）
        WaitForSingleObject(para->p->bufferMutex, INFINITE);

        // 客户端退出，关闭该客户端的套接字，释放相应资源
        if (recvMsg == "quit" || ret < 0)
        {
            // 删除 socket 对应的同步信号量, 释放 socket 和同步信号的映射
            // 注意这里顺序不能换
            CloseHandle(para->p->cliSockSyncMap[para->sock]);
            para->p->cliSockSyncMap.erase(para->sock);

            // 释放 ID 和 socket 的映射
            for (IdSockMap::iterator iter = para->p->cliSockMap.begin(); iter != para->p->cliSockMap.end(); iter++)
            {
                if (iter->second == para->sock)
                {
                    para->p->cliSockMap.erase(iter);
                    break;
                }
            }

            // 关闭客户端 socket
            closesocket(para->sock);
            std::cout << std::endl << "Attention: A Client has leave..." << std::endl;

            // signal (bufferMutex)
            ReleaseSemaphore(para->p->bufferMutex, 1, NULL);

            // 客户端退出，其对应的线程也结束
            // 结束发送消息线程
            para->p->killThrd[para->sock] = true;
            // 不要忘记结束本线程循环！
            break;
        }

        std::cout << "socket " << para->sock << " says: " << recvMsg << std::endl;
        // signal (bufferMutex)
        ReleaseSemaphore(para->p->bufferMutex, 1, NULL);
    }
    return 0;
}

void SocketServer::Accept()
{
    // 不断等待客户端请求的到来
    while (true)
    {
        // 收到一个客户端的连接
        SOCKET sockConn = accept(srvSocket, NULL, NULL);
        if (SOCKET_ERROR == sockConn)
        {
            WaitForSingleObject(bufferMutex, INFINITE);
            error(SocketServer::ERR_STA::accept_SOCKET_ERROR);
            ReleaseSemaphore(bufferMutex, 1, NULL);
            continue;
        }

        // wait (bufferMutex)
        WaitForSingleObject(bufferMutex, INFINITE);

        // 向客户端询问 ID
        Send(sockConn, "请输入您的 ID：");
        ClientSockID cliID;
        Recv(sockConn, cliID);

        // 判断 ID 的格式是否正确


        if (cliSockMap.find(cliID) != cliSockMap.end())
        {
            // ID 已经存在，错误处理并关闭连接、释放资源
            error(SocketServer::ERR_STA::cli_ID_duplicated_ERROR);
            closesocket(sockConn);
            ReleaseSemaphore(bufferMutex, 1, NULL);
            continue;
        }
        else
        {
            ReleaseSemaphore(bufferMutex, 1, NULL);
            // 创建客户端 ID 和其 socket 之间的一一映射
            cliSockMap.insert(IdSockPair(cliID, sockConn));
        }


        // 创建 socket 和发送线程同步信号量之间的一一映射
        HANDLE sendSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
        cliSockSyncMap.insert(SockHandlePair(sockConn, sendSemaphore));

        // 创建 socket 和管理线程关闭的标志之间的一一映射
        killThrd.insert(SockFlagPair(sockConn, false));

        // 构造线程参数的结构体
        Para* para = new Para;
        para->p = (SocketServer*)this;
        para->sock = sockConn;

        // 创建线程，用来接收该客户端的信息
        HANDLE receiveThread = CreateThread(NULL, 0, ReceiveMessageThread, (LPVOID)para, 0, NULL);

        // 创建线程，用来向该客户端发送消息
        HANDLE specSendThread = CreateThread(NULL, 0, CliSendMessageThread, (LPVOID)para, 0, NULL);

        // wait (bufferMutex)
        WaitForSingleObject(bufferMutex, INFINITE);
        if (NULL == receiveThread || NULL == specSendThread)
        {
            // 线程创建失败，错误处理并关闭连接
            error(SocketServer::ERR_STA::create_thread_ERROR);
            ReleaseSemaphore(bufferMutex, 1, NULL);
            closesocket(sockConn);
            if (receiveThread)
                CloseHandle(receiveThread);
            if (specSendThread)
                CloseHandle(specSendThread);
            continue;
        }
        else
        {
            // 线程创建成功
            std::cout << std::endl << "Create Receive Client Thread OK." << std::endl;
        }

        // signal (bufferMutex)
        ReleaseSemaphore(bufferMutex, 1, NULL);

        // 释放句柄
        CloseHandle(receiveThread);
        CloseHandle(specSendThread);
    }
}

void SocketServer::waitThread()
{
    // 等待线程结束
    WaitForSingleObject(sendManagerThread, INFINITE);
}
