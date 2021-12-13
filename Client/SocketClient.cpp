#include "SocketClient.h"

SocketClient::SocketClient()
{
    initSocketClient();
    Send("\nAttention: A Client has enter...\n");

    bufferMutex = CreateSemaphore(NULL, 1, 1, NULL);

    sendThread = CreateThread(NULL, 0, SendMessageThread, this, 0, NULL);
    receiveThread = CreateThread(NULL, 0, ReceiveMessageThread, this, 0, NULL);

    std::cout << "create thread over" << std::endl;
}

void SocketClient::createThread()
{

}

void SocketClient::waitThread()
{
    WaitForSingleObject(sendThread, INFINITE);  // �ȴ��߳̽���
}

SocketClient::~SocketClient()
{
    if (cliSocket)
        closesocket(cliSocket);
    CloseHandle(sendThread);
    CloseHandle(receiveThread);
    CloseHandle(bufferMutex);

    printf("End linking...\n");
    printf("\n");
}

void SocketClient::error(SocketClient::ERR_STA err)
{

    std::cout << "error: " << magic_enum::enum_name(err) << std::endl;
}

void SocketClient::initSocketClient()
{
    int iRet = 0;

    // ���� 2.2 �汾���׽��ֿ�
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

    // ������ʽ�׽���
    cliSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (cliSocket == INVALID_SOCKET)
    {
        error(SocketClient::ERR_STA::socket_INVALID_SOCKET);
        return;
    }
    std::cout << "done! create srvSocket." << std::endl;

    //��ʼ���������˵�ַ�����
    SOCKADDR_IN srvAddr;
    //srvAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    inet_pton(AF_INET, "127.0.0.1", &srvAddr.sin_addr.S_un.S_addr);
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(6000);

    //���ӷ�����
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
    // �����ڴ�ռ�: ���ݳ��� + ��ͷ4�ֽ�(�洢���ݳ���)
    char* sendPackage = new char[msg.size() + 4];
    int bigLen = htonl(msg.size());
    memcpy(sendPackage, &bigLen, 4);
    memcpy(sendPackage + 4, msg.data(), msg.size());

    // ��������
    int ret = writen(sendPackage, msg.size() + 4);
    delete[] sendPackage;
    return ret;

    /*send(cliSocket, sendPackage, strlen(sendPackage) + 1, 0);
    std::cout << "send: " << msg << std::endl;*/
}

int SocketClient::Recv(std::string& msg)
{
    // ������ͷ
    int len = 0;
    readn((char*)&len, 4);
    len = ntohl(len);
    std::cout << "���ݿ��С: " << len << std::endl;

    // ���ݶ����ĳ��ȷ����ڴ�
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

DWORD WINAPI SocketClient::SendMessageThread(LPVOID IpParameter)
{
    SocketClient* p = (SocketClient*)IpParameter;
    while (1) {
        std::string msg;
        std::getline(std::cin, msg);
        WaitForSingleObject(p->bufferMutex, INFINITE);     // P����Դδ��ռ�ã�
        //if ("quit" == msg) {
        //    msg.push_back('\0');
        //    //          send(sockClient, msg.c_str(), msg.size(), 0);
        //    send(cliSocket, msg.c_str(), 200, 0);
        //    break;
        //}
        //else {
        //    msg.append("\n");
        //}
        //printf("\nI Say:(\"quit\"to exit):");
        if (msg == "quit")
            break;
        std::cout << std::endl << "I Say:(\"quit\"to exit):" << msg << std::endl;

        //std::cout << msg;
        //  send(sockClient, msg.c_str(), msg.size(), 0); // ������Ϣ
        //send(cliSocket, msg.c_str(), 200, 0); // ������Ϣ

        p->Send(msg);
        ReleaseSemaphore(p->bufferMutex, 1, NULL);     // V����Դռ����ϣ�
    }
    return 0;
}

DWORD WINAPI SocketClient::ReceiveMessageThread(LPVOID IpParameter)
{
    SocketClient* p = (SocketClient*)IpParameter;
    while (1) {
        //char recvBuf[300];
        //recv(cliSocket, recvBuf, 200, 0);

        std::string recvMsg;
        p->Recv(recvMsg);
        WaitForSingleObject(p->bufferMutex, INFINITE);     // P����Դδ��ռ�ã�

        //printf("%s Says: %s\n", "Server", recvBuf);     // ������Ϣ
        std::cout << "Server says: " << recvMsg << std::endl;
        ReleaseSemaphore(p->bufferMutex, 1, NULL);     // V����Դռ����ϣ�
    }
    return 0;
}