#include "SocketServer.h"

const char* RSL_PATH = "./scripts/complete-1.rsl";
//const char* RSL_PATH = "./scripts/complete-2.rsl";
//const char* RSL_PATH = "./scripts/complete-3.rsl";

SocketServer::SocketServer(int connectNumMax_ = 10)
    : connectNumMax(connectNumMax_), scriptSrv(RSL_PATH)
{
#ifdef GTEST

    dbg.anaLogName(RSL_PATH, logName);

#endif // GTEST

    if (scriptSrv.init())
        dbg.out("script server ready!\n", logName);
    else
    {
        dbg.out("script server init failed.\nplease check the log.\n", logName);
        exit(0);
    }
    initSocketServer();

    bufferMutex = CreateSemaphore(NULL, 1, 1, NULL);
    allKill = CreateSemaphore(NULL, 0, 1, NULL);
    sendSync = CreateSemaphore(NULL, 1, 1, NULL);
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
}


void SocketServer::error(SocketServer::ERR_STA err)
{
    dbg.out("error: " + std::string(magic_enum::enum_name(err)) + "\n", logName);
}


void SocketServer::initSocketServer()
{
    int iRet = 0;

    dbg.out("====================== Server ==========================\n", logName);

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
    dbg.out("done! load the socket data.\n", logName);


    // 创建流式套接字
    srvSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (srvSocket == INVALID_SOCKET)
    {
        error(SocketServer::ERR_STA::socket_INVALID_SOCKET);
        return;
    }
    dbg.out("done! create srvSocket.\n", logName);

    // 初始化服务器地址族变量
    srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(6000);
    dbg.out("done! init server address.\n", logName);

    // 绑定
    iRet = bind(srvSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
    if (iRet == SOCKET_ERROR)
    {
        error(SocketServer::ERR_STA::bind_SOCKET_ERROR);
        return;
    }
    dbg.out("done! bind.\n", logName);

    //监听
    iRet = listen(srvSocket, connectNumMax);
    if (iRet == SOCKET_ERROR)
    {
        error(SocketServer::ERR_STA::listen_SOCKET_ERROR);
        return;
    }
    dbg.out("done! listen.\n", logName);
    dbg.out("Server ready!\n", logName);
    dbg.out("\n===================== by nanyf ==========================\n\n", logName);
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


DWORD WINAPI SocketServer::CliSendMessageThread(LPVOID IpParameter)
{
    // 将传入参数转为原来的结构体类型
    Para* para = (Para*)IpParameter;

    while (1)
    {
        // kill 标志为真，则直接退出循环，结束线程
        if (para->p->killThrd[para->sock])
            break;

        std::string msg;
        int ret;

        // 根据 scriptSrv 的指示执行不同的任务
        ScriptServer::INTERPRET_STA todo = para->p->scriptSrv.srvInterpret(para->id);
        switch (todo)
        {
        case ScriptServer::INTERPRET_STA::Do:
            break;
        case ScriptServer::INTERPRET_STA::Listen:
            // 置为监听状态
            para->p->listenValid[para->sock] = true;
            WaitForSingleObject(para->p->listenSync[para->sock], INFINITE);
            // 监听到消息后，关闭监听状态
            para->p->listenValid[para->sock] = false;
            // 设置步骤的完成状态
            para->p->scriptSrv.setFinished(para->id);
            break;
        case ScriptServer::INTERPRET_STA::Exit:
            para->p->killThrd[para->sock] = true;
            break;
        case ScriptServer::INTERPRET_STA::Out:
            para->p->scriptSrv.getOutputMsg(para->id, msg);
            // wait (bufferMutex)
            WaitForSingleObject(para->p->bufferMutex, INFINITE);

            // 向指定的客户端发送消息
            ret = para->p->Send(para->sock, msg);
            if (ret < 0)
                para->p->dbg.out("send error;\n", para->p->logName);
            else
                para->p->dbg.out("I say: " + msg + "\n", para->p->logName);

            // signal (bufferMutex)
            ReleaseSemaphore(para->p->bufferMutex, 1, NULL);

            // 设置步骤的完成状态
            para->p->scriptSrv.setFinished(para->id);
            break;
        default:
            break;
        }
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
        if (recvMsg == "quit" || ret < 0 || para->p->killThrd[para->sock])
        {
            // 删除用户
            para->p->scriptSrv.deleteUser(para->id);

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
            para->p->dbg.out("Attention: A Client has leave...\n", para->p->logName);
            //std::cout << std::endl << "" << std::endl;


            // signal (bufferMutex)
            ReleaseSemaphore(para->p->bufferMutex, 1, NULL);

            // 客户端退出，其对应的线程也结束
            // 结束发送消息线程
            para->p->killThrd[para->sock] = true;
            // 不要忘记结束本线程循环！
            break;
        }

        // 当服务器在 listen 的时候，将接收到的消息保存到 user 中
        if (para->p->listenValid[para->sock])
        {
            para->p->scriptSrv.msgToUserInputKey(para->id, recvMsg);
            ReleaseSemaphore(para->p->listenSync[para->sock], 1, NULL);
        }
        para->p->dbg.out("socket " + std::to_string(para->sock) + " says: " + recvMsg + "\n", para->p->logName);

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

        if (cliSockMap.find(cliID) != cliSockMap.end())
        {
            // ID 已经存在，错误处理并关闭连接、释放资源
            error(SocketServer::ERR_STA::cli_ID_duplicated_ERROR);
            closesocket(sockConn);
            ReleaseSemaphore(bufferMutex, 1, NULL);
            continue;
        }

        // 创建一个用户
        if (scriptSrv.createUser(cliID) != ScriptServer::NEW_USER_STA::Succeed)
        {
            // 创建不成功，关闭连接、释放资源
            closesocket(sockConn);
            ReleaseSemaphore(bufferMutex, 1, NULL);
            continue;
        }
        else
        {
            // 创建客户端 ID 和其 socket 之间的一一映射
            cliSockMap.insert(IdSockPair(cliID, sockConn));
        }


        // 创建 socket 和发送线程同步信号量之间的一一映射
        HANDLE sendSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
        cliSockSyncMap.insert(SockHandlePair(sockConn, sendSemaphore));

        // 创建 socket 和用于 listen 的信号量之间的一一映射
        HANDLE listenSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
        listenSync.insert(SockHandlePair(sockConn, listenSemaphore));

        // 创建 socket 和标志 listen 是否有效的量之间的一一映射
        listenValid.insert(SockFlagPair(sockConn, false));

        // 创建 socket 和管理线程关闭的标志之间的一一映射
        killThrd.insert(SockFlagPair(sockConn, false));

        // 构造线程参数的结构体
        Para* para = new Para;
        para->p = (SocketServer*)this;
        para->sock = sockConn;
        para->id = cliID;

        // 创建线程，用来接收该客户端的信息
        HANDLE receiveThread = CreateThread(NULL, 0, ReceiveMessageThread, (LPVOID)para, 0, NULL);

        // 创建线程，用来向该客户端发送消息
        HANDLE specSendThread = CreateThread(NULL, 0, CliSendMessageThread, (LPVOID)para, 0, NULL);

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
            dbg.out("Create Receive Client Thread OK.\n", logName);
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
    WaitForSingleObject(allKill, INFINITE);
}
