#include "SocketServer.h"

SocketServer::SocketServer(int connectNumMax_ = 10)
    : connectNumMax(connectNumMax_)
{
    initSocketServer();

    bufferMutex = CreateSemaphore(NULL, 1, 1, NULL);
    sendSync = CreateSemaphore(NULL, 1, 1, NULL);
    //sendThread = CreateThread(NULL, 0, CliSendMessageThread, this, 0, NULL);
    sendManagerThread = CreateThread(NULL, 0, ManageSendThread, this, 0, NULL);
}

//void SocketServer::init()
//{
//
//}

SocketServer::~SocketServer()
{
    if (srvSocket)
        closesocket(srvSocket);

    // 删除连接到的客户 Socket
    //for (int i = 0; i < cliSockMap.size(); ++i)
    //{
    //    if (clientSocketGroup[i])
    //        closesocket(clientSocketGroup[i]);
    //}
    for (IdSockMap::iterator iter = cliSockMap.begin(); iter != cliSockMap.end(); iter++)
    {
        if (iter->second)
            closesocket(iter->second);
    }

    //CloseHandle(sendThread);
    CloseHandle(bufferMutex);
}

void SocketServer::error(SocketServer::ERR_STA err)
{

    std::cout << "error: " << magic_enum::enum_name(err) << std::endl;
}

void SocketServer::initSocketServer()
{
    int iRet = 0;

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

}

//void SocketServer::Accept()
//{
//    int len = sizeof(SOCKADDR);
//    //accSocket = accept(srvSocket, (SOCKADDR*)&clientAddr, &len);
//    if (accSocket == SOCKET_ERROR)
//    {
//        error(SocketServer::ERR_STA::accept_SOCKET_ERROR);
//        return;
//    }
//}


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
    readn(srcSocket, (char*)&len, 4);
    len = ntohl(len);
    std::cout << "数据块大小: " << len << std::endl;

    // 根据读出的长度分配内存
    char* buf = new char[len + 1];
    int ret = readn(srcSocket, buf, len);
    if (ret != len)
    {
        msg = "";
        return -1;
    }
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
    SocketServer* p = (SocketServer*)IpParameter;

    while (1)
    {
        WaitForSingleObject(p->sendSync, INFINITE);

        std::cout << "请输入目的客户端 ID: ";
        std::string dstID;
        std::getline(std::cin, dstID);

        // P（资源未被占用）
        WaitForSingleObject(p->bufferMutex, INFINITE);

        if (p->cliSockMap.find(dstID) == p->cliSockMap.end())
        {
            // 目的 ID 不存在，错误处理并释放资源
            p->error(SocketServer::ERR_STA::cli_ID_not_exist_ERROR);
            ReleaseSemaphore(p->bufferMutex, 1, NULL);
            ReleaseSemaphore(p->sendSync, 1, NULL);
            continue;
        }

        SOCKET dstSock = p->cliSockMap[dstID];

        std::cout << "dstSock: " << dstSock << std::endl;

        if (p->cliSockSyncMap.find(dstSock) != p->cliSockSyncMap.end())
        {
            ReleaseSemaphore(p->bufferMutex, 1, NULL);
            ReleaseSemaphore(p->cliSockSyncMap[dstSock], 1, NULL);
        }
        else
        {
            p->error(SocketServer::ERR_STA::no_socket_ERROR);
            ReleaseSemaphore(p->sendSync, 1, NULL);
            ReleaseSemaphore(p->bufferMutex, 1, NULL);
        }
    }
}

DWORD WINAPI SocketServer::CliSendMessageThread(LPVOID IpParameter)
{
    struct Para* para = (struct Para*)IpParameter;

    while (1)
    {
        // wait (sync)
        if (para->p->cliSockSyncMap.find(para->sock) != para->p->cliSockSyncMap.end())
            WaitForSingleObject(para->p->cliSockSyncMap[para->sock], INFINITE);
        else
            continue;

        std::string msg;
        std::getline(std::cin, msg);

        // P（资源未被占用）
        WaitForSingleObject(para->p->bufferMutex, INFINITE);

        //if (para->p->cliSockMap.find(dstID) == para->p->cliSockMap.end())
        //{
        //    para->p->error(SocketServer::ERR_STA::cli_ID_not_exist_ERROR);
        //    // V（资源占用完毕）
        //    ReleaseSemaphore(para->p->bufferMutex, 1, NULL);
        //    continue;
        //}

        int ret = 0;
        //// 向所有的客户端发送同样的消息
        //for (int i = 0; i < p->clientSocketGroup.size(); ++i)
        //{
        //    if ((ret = p->Send(p->clientSocketGroup[i], msg)) < 0)
        //        break;
        //}

        // 向指定的客户端发送消息

        ret = para->p->Send(para->sock, msg);


        if (ret < 0)
            std::cout << "send error;" << std::endl;
        else
            std::cout << "I say: " << msg << std::endl;
        // V（资源占用完毕）
        ReleaseSemaphore(para->p->bufferMutex, 1, NULL);

        ReleaseSemaphore(para->p->sendSync, 1, NULL);
    }
    return 0;
}


DWORD WINAPI SocketServer::ReceiveMessageThread(LPVOID IpParameter)
{
    struct Para* para = (struct Para*)IpParameter;

    while (1)
    {
        std::string recvMsg;
        para->p->Recv(para->sock, recvMsg);

        // P（资源未被占用）
        WaitForSingleObject(para->p->bufferMutex, INFINITE);
        // 客户端退出，关闭该客户端的套接字，释放相应资源
        if (recvMsg == "quit")
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

            closesocket(para->sock);
            std::cout << std::endl << "Attention: A Client has leave..." << std::endl;
            // V（资源占用完毕）
            ReleaseSemaphore(para->p->bufferMutex, 1, NULL);
            break;
        }

        std::cout << "One Client Says: " << recvMsg << std::endl;
        // V（资源占用完毕）
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
        //clientSocketGroup.push_back(sockConn);


        WaitForSingleObject(bufferMutex, INFINITE);

        // 向客户端询问 ID
        Send(sockConn, "请输入您的 ID：");
        ClientSockID cliID;
        Recv(sockConn, cliID);
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


        // 创建 socket 和同步信号量之间的一一映射
        HANDLE sync = CreateSemaphore(NULL, 0, 1, NULL);
        cliSockSyncMap.insert(SockSyncPair(sockConn, sync));

        // 构造线程参数的结构体
        struct Para para;
        para.p = (SocketServer*)this;
        para.sock = sockConn;

        // 创建线程，用来接收该客户端的信息
        HANDLE receiveThread = CreateThread(NULL, 0, ReceiveMessageThread, (LPVOID)&para, 0, NULL);
        //recvThreads.push_back(receiveThread);

        // 创建线程，用来向该客户端发送消息
        HANDLE specSendThread = CreateThread(NULL, 0, CliSendMessageThread, (LPVOID)&para, 0, NULL);

        // P（资源未被占用）
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
            std::cout << std::endl << "Create Receive Client Thread OK." << std::endl;
            //cliSendThreads.push_back(specSendThread);
        }
        //
        //if (NULL == sendThread)
        //{
        //    //std::cout << std::endl << "CreatThread AnswerThread() failed." << std::endl;
        //    // 线程创建失败，错误处理并关闭连接
        //    error(SocketServer::ERR_STA::create_thread_ERROR);
        //    closesocket(sockConn);
        //    continue;
        //}
        //else
        //    std::cout << std::endl << "Create Receive Client Thread OK." << std::endl;



        // 至此，连接建立成功，收发消息的线程建立成功


        // V（资源占用完毕）
        ReleaseSemaphore(bufferMutex, 1, NULL);

        CloseHandle(receiveThread);
        CloseHandle(specSendThread);
    }
}

void SocketServer::waitThread()
{
    // 等待线程结束
    WaitForSingleObject(sendManagerThread, INFINITE);
}
