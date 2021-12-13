#include "SocketClient.h"

SocketClient::SocketClient()
{
    initSocketClient();
}
SocketClient::~SocketClient()
{
    if (cliSocket)
        closesocket(cliSocket);
}

void SocketClient::error(SocketClient::ERR_STA err)
{

    std::cout << "error: " << magic_enum::enum_name(err) << std::endl;
}

void SocketClient::initSocketClient()
{
    int iRet = 0;

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
    std::cout << "client ready. now you can send messages." << std::endl;

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

    /*send(cliSocket, sendPackage, strlen(sendPackage) + 1, 0);
    std::cout << "send: " << msg << std::endl;*/
}

int SocketClient::Recv(std::string& msg)
{
    // 读数据头
    int len = 0;
    readn((char*)&len, 4);
    len = ntohl(len);
    std::cout << "数据块大小: " << len << std::endl;

    // 根据读出的长度分配内存
    char* buf = new char[len + 1];
    int ret = readn(buf, len);
    if (ret != len)
    {
        msg = "";
        return -1;
    }
    buf[len] = '\0';
    msg = std::string(buf);
    delete[] buf;

    return len;

    //char* recvBuf = new char[bufLen + 1];
    //recv(cliSocket, recvBuf, bufLen, 0);
    //buf = std::string(recvBuf);
    //std::cout << "recv: " << buf << std::endl;
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