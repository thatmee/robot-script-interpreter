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
    // �ȴ��߳̽���
    WaitForSingleObject(receiveThread, INFINITE);
}

SocketClient::~SocketClient()
{
    // �ر� socket
    if (cliSocket)
        closesocket(cliSocket);

    // �ͷ����о��
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
    std::cout << "client ready!" << std::endl;
    std::cout << std::endl << "===================== by nanyf ==========================" << std::endl << std::endl;
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
}

int SocketClient::Recv(std::string& msg)
{
    // ������ͷ
    int len = 0;
    int ret = readn((char*)&len, 4);
    if (ret == -1)
    {
        // û�ж�������ͷ���Զ��쳣
        msg = "";
        return -1;
    }
    len = ntohl(len);

    // ���ݶ����ĳ��ȷ����ڴ�
    char* buf = new char[len + 1];
    ret = readn(buf, len);
    if (ret != len)
    {
        msg = "";
        delete[] buf;
        return -1;
    }

    // ���ݶ����ĳ��������ַ�����β
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
    // ��������� this ָ��
    SocketClient* p = (SocketClient*)IpParameter;

    // ѭ���������л�ȡ���룬�����͵�������
    while (1) {
        // �������ж�ȡ��������
        std::string msg;
        std::getline(std::cin, msg);

        // �˳��̱߳�־Ϊ�棬ֱ�ӽ���ѭ��
        if (p->killThrd)
            break;

        // wait (bufferMutex)
        WaitForSingleObject(p->bufferMutex, INFINITE);
        std::cout << std::endl << "I Say:(\"quit\"to exit):" << msg << std::endl;
        p->Send(msg);

        // signal (bufferMutex)
        ReleaseSemaphore(p->bufferMutex, 1, NULL);

        // ���� quit �˳�
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
    // ��������� this ָ��
    SocketClient* p = (SocketClient*)IpParameter;

    // ѭ���ӷ�������ȡ��Ϣ������ʾ�������д���
    while (1) {
        // ��������
        std::string recvMsg;
        int ret = p->Recv(recvMsg);

        // �˳��̱߳�־Ϊ�棬ֱ�ӽ���ѭ��
        if (p->killThrd)
            break;

        // ����ֵ�쳣���Զ����Ӵ���ֱ���˳�
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