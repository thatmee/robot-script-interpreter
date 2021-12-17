#include "SocketClient.h"

SocketClient::SocketClient()
{
    initSocketClient();

    bufferMutex = CreateSemaphore(NULL, 1, 1, NULL);
    sendThread = CreateThread(NULL, 0, SendMessageThread, this, 0, NULL);
    receiveThread = CreateThread(NULL, 0, ReceiveMessageThread, this, 0, NULL);
    killThrd = false;
}

void SocketClient::waitThread()
{
    // 等待线程结束
    WaitForSingleObject(receiveThread, INFINITE);
}

SocketClient::~SocketClient()
{
    // 关闭 socket
    if (cliSocket)
        closesocket(cliSocket);

    // 释放所有句柄
    CloseHandle(sendThread);
    CloseHandle(receiveThread);
    CloseHandle(bufferMutex);
}

void SocketClient::error(SocketClient::ERR_STA err)
{

    std::cout << "error: " << magic_enum::enum_name(err) << std::endl;
}

void SocketClient::initSocketClient()
{
    int iRet = 0;

    std::cout << "====================== Client ==========================" << std::endl;

    // 加载 2.2 版本的套接字库
    iRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iRet != 0)
    {
        error(SocketClient::ERR_STA::WSAStartup_ReturnNoneZero);
        return;
    }
    if (2 != LOBYTE(wsaData.wVersion) || 2 != HIBYTE(wsaData.wVersion))
    {
        WSACleanup();
        error(SocketClient::ERR_STA::WSADATA_versionWrong);
        return;
    }
    std::cout << "done! load the socket data." << std::endl;

    // 创建流式套接字
    cliSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (cliSocket == INVALID_SOCKET)
    {
        error(SocketClient::ERR_STA::socket_INVALID_SOCKET);
        return;
    }
    std::cout << "done! create srvSocket." << std::endl;

    //初始化服务器端地址族变量
    SOCKADDR_IN srvAddr;
    //srvAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    inet_pton(AF_INET, "127.0.0.1", &srvAddr.sin_addr.S_un.S_addr);
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(6000);

    //连接服务器
    iRet = connect(cliSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
    if (0 != iRet)
    {
        error(SocketClient::ERR_STA::connect_ERROR);
        return;
    }
    std::cout << "done! connect to server." << std::endl;
    std::cout << "client ready!" << std::endl;
    std::cout << std::endl << "===================== by nanyf ==========================" << std::endl << std::endl;
}

int SocketClient::Send(std::string msg)
{
    // 申请内存空间: 数据长度 + 包头4字节(存储数据长度)
    char* sendPackage = new char[msg.size() + 4];
    int bigLen = htonl(msg.size());
    memcpy(sendPackage, &bigLen, 4);
    memcpy(sendPackage + 4, msg.data(), msg.size());

    // 发送数据
    int ret = writen(sendPackage, msg.size() + 4);
    delete[] sendPackage;
    return ret;
}

int SocketClient::Recv(std::string& msg)
{
    // 读数据头
    int len = 0;
    int ret = readn((char*)&len, 4);
    if (ret == -1)
    {
        // 没有读到数据头，对端异常
        msg = "";
        return -1;
    }
    len = ntohl(len);

    // 根据读出的长度分配内存
    char* buf = new char[len + 1];
    ret = readn(buf, len);
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

int SocketClient::readn(char* buf, int size)
{
    int nread = 0;
    int left = size;
    char* p = buf;

    while (left > 0)
    {
        if ((nread = recv(cliSocket, p, left, 0)) > 0)
        {
            p += nread;
            left -= nread;
        }
        else if (nread == -1)
        {
            return -1;
        }
    }
    return size;
}

int SocketClient::writen(const char* msg, int size)
{
    int left = size;
    int nwrite = 0;
    const char* p = msg;

    while (left > 0)
    {
        if ((nwrite = send(cliSocket, msg, left, 0)) > 0)
        {
            p += nwrite;
            left -= nwrite;
        }
        else if (nwrite == -1)
        {
            return -1;
        }
    }
    return size;
}

DWORD WINAPI SocketClient::SendMessageThread(LPVOID IpParameter)
{
    // 传入了类的 this 指针
    SocketClient* p = (SocketClient*)IpParameter;

    // 循环从命令行获取输入，并发送到服务器
    while (1) {
        // 从命令行读取发送数据
        std::string msg;
        std::getline(std::cin, msg);

        // 退出线程标志为真，直接结束循环
        if (p->killThrd)
            break;

        // wait (bufferMutex)
        WaitForSingleObject(p->bufferMutex, INFINITE);
        std::cout << std::endl << "I Say:(\"quit\"to exit):" << msg << std::endl;
        p->Send(msg);

        // signal (bufferMutex)
        ReleaseSemaphore(p->bufferMutex, 1, NULL);

        // 输入 quit 退出
        if (msg == "quit")
        {
            p->killThrd = true;
            break;
        }
    }
    return 0;
}

DWORD WINAPI SocketClient::ReceiveMessageThread(LPVOID IpParameter)
{
    // 传入了类的 this 指针
    SocketClient* p = (SocketClient*)IpParameter;

    // 循环从服务器获取消息，并显示到命令行窗口
    while (1) {
        // 接收数据
        std::string recvMsg;
        int ret = p->Recv(recvMsg);

        // 退出线程标志为真，直接结束循环
        if (p->killThrd)
            break;

        // 返回值异常，对端连接错误，直接退出
        if (ret < 0)
        {
            closesocket(p->cliSocket);
            p->killThrd = true;
            break;
        }

        // wait (bufferMutex)
        WaitForSingleObject(p->bufferMutex, INFINITE);
        std::cout << "Server says: " << recvMsg << std::endl;
        // signal (bufferMutex)
        ReleaseSemaphore(p->bufferMutex, 1, NULL);
    }
    return 0;
}