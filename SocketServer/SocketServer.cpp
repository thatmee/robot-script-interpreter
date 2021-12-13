#include "SocketServer.h"

SocketServer::SocketServer(int connectNumMax_ = 10)
    : connectNumMax(connectNumMax_)
{
    initSocketServer();

    bufferMutex = CreateSemaphore(NULL, 1, 1, NULL);
    sendThread = CreateThread(NULL, 0, SendMessageThread, this, 0, NULL);
}

SocketServer::~SocketServer() {
    if (srvSocket)
        closesocket(srvSocket);
    if (accSocket)
        closesocket(accSocket);
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

void SocketServer::Accept()
{
    int len = sizeof(SOCKADDR);
    accSocket = accept(srvSocket, (SOCKADDR*)&clientAddr, &len);
    if (accSocket == SOCKET_ERROR)
    {
        error(SocketServer::ERR_STA::accept_SOCKET_ERROR);
        return;
    }
}


//void SocketServer::Send(std::string buf, const int bufLen)
//{
//    char* sendBuf = buf.data();
//    send(accSocket, sendBuf, strlen(sendBuf) + 1, 0);
//    std::cout << "send: " << buf << std::endl;
//}
//
//void SocketServer::Recv(std::string& buf, const int bufLen)
//{
//    char* recvBuf = new char[bufLen + 1];
//    recv(accSocket, recvBuf, bufLen, 0);
//    buf = std::string(recvBuf);
//    std::cout << "recv: " << buf << std::endl;
//}

int SocketServer::Send(std::string msg)
{
    // 申请内存空间: 数据长度 + 包头4字节(存储数据长度)
    char* sendPackage = new char[msg.size() + 4];
    int bigLen = htonl(msg.size());
    memcpy(sendPackage, &bigLen, 4);
    memcpy(sendPackage + 4, msg.data(), msg.size());

    // 发送数据
    int ret = 0;
    for (int i = 0; i < clientSocketGroup.size(); ++i)
    {
        ret = writen(clientSocketGroup[i], sendPackage, msg.size() + 4);
    }

    //int ret = writen(sendPackage, msg.size() + 4);
    delete[] sendPackage;
    return ret;

    /*send(cliSocket, sendPackage, strlen(sendPackage) + 1, 0);
    std::cout << "send: " << msg << std::endl;*/
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

    //char* recvBuf = new char[bufLen + 1];
    //recv(cliSocket, recvBuf, bufLen, 0);
    //buf = std::string(recvBuf);
    //std::cout << "recv: " << buf << std::endl;
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
        {
            return -1;
        }
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
        {
            return -1;
        }
    }
    return size;
}

DWORD WINAPI SocketServer::SendMessageThread(LPVOID IpParameter)
{
    SocketServer* p = (SocketServer*)IpParameter;
    while (1) {
        std::string msg;
        std::getline(std::cin, msg);
        WaitForSingleObject(p->bufferMutex, INFINITE);     // P（资源未被占用）
    /*  if("quit" == msg){
            ReleaseSemaphore(bufferMutex, 1, NULL);     // V（资源占用完毕）
            return 0;
        }
        else*/
        /*{
            msg.append("\n");
        }*/
        /*printf("I Say:(\"quit\"to exit):");
        cout << msg;*/

        std::cout << "I Say: " << msg << std::endl;
        p->Send(msg);
        ReleaseSemaphore(p->bufferMutex, 1, NULL);     // V（资源占用完毕）
    }
    return 0;
}


DWORD WINAPI SocketServer::ReceiveMessageThread(LPVOID IpParameter)
{
    SOCKET ClientSocket = (SOCKET)(LPVOID)IpParameter;
    while (1) {
        char recvBuf[300];
        recv(ClientSocket, recvBuf, 200, 0);
        WaitForSingleObject(bufferMutex, INFINITE);     // P（资源未被占用）

        if (recvBuf[0] == 'q' && recvBuf[1] == 'u' && recvBuf[2] == 'i' && recvBuf[3] == 't' && recvBuf[4] == '\0') {
            vector<SOCKET>::iterator result = find(clientSocketGroup.begin(), clientSocketGroup.end(), ClientSocket);
            clientSocketGroup.erase(result);
            closesocket(ClientSocket);
            ReleaseSemaphore(bufferMutex, 1, NULL);     // V（资源占用完毕）
            printf("\nAttention: A Client has leave...\n");
            break;
        }

        printf("%s Says: %s\n", "One Client", recvBuf);     // 接收信息

        ReleaseSemaphore(bufferMutex, 1, NULL);     // V（资源占用完毕）
    }
    return 0;
}