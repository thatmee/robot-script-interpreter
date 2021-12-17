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
    // �ر����ӵ��Ŀͻ� Socket
    for (IdSockMap::iterator iter = cliSockMap.begin(); iter != cliSockMap.end(); iter++)
    {
        if (iter->second)
            closesocket(iter->second);
    }

    // �رշ������˼��� socket
    if (srvSocket)
        closesocket(srvSocket);

    // �ͷ����о��
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

    // ���� 2.2 �汾���׽��ֿ�
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

    // ������ʽ�׽���
    srvSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (srvSocket == INVALID_SOCKET)
    {
        error(SocketServer::ERR_STA::socket_INVALID_SOCKET);
        return;
    }
    std::cout << "done! create srvSocket." << std::endl;

    // ��ʼ����������ַ�����
    srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(6000);
    std::cout << "done! init server address." << std::endl;

    // ��
    iRet = bind(srvSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
    if (iRet == SOCKET_ERROR)
    {
        error(SocketServer::ERR_STA::bind_SOCKET_ERROR);
        return;
    }
    std::cout << "done! bind." << std::endl;


    //����
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
    // �����ڴ�ռ�: ���ݳ��� + ��ͷ4�ֽ�(�洢���ݳ���)
    char* sendPackage = new char[msg.size() + 4];
    int bigLen = htonl(msg.size());
    memcpy(sendPackage, &bigLen, 4);
    memcpy(sendPackage + 4, msg.data(), msg.size());

    // ��������
    int ret = writen(dstSocket, sendPackage, msg.size() + 4);
    delete[] sendPackage;
    return ret;
}

int SocketServer::Recv(SOCKET srcSocket, std::string& msg)
{
    // ������ͷ
    int len = 0;
    int ret = readn(srcSocket, (char*)&len, 4);
    if (ret == -1)
    {
        // û�ж�������ͷ���Զ��쳣
        msg = "";
        return -1;
    }
    len = ntohl(len);

    // ���ݶ����ĳ��ȷ����ڴ�
    char* buf = new char[len + 1];
    ret = readn(srcSocket, buf, len);
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
    // ��������� this ָ��
    SocketServer* p = (SocketServer*)IpParameter;

    while (1)
    {
        // ��һ����Ϣ���롢������Ϻ�Ŵ�����һ����������
        WaitForSingleObject(p->sendSync, INFINITE);

        std::cout << "������Ŀ�Ŀͻ��� ID: ";
        std::string dstID;
        std::getline(std::cin, dstID);

        // wait (bufferMutex)
        WaitForSingleObject(p->bufferMutex, INFINITE);

        if (p->cliSockMap.find(dstID) == p->cliSockMap.end())
        {
            // Ŀ�� ID �����ڣ��������ͷ���Դ
            p->error(SocketServer::ERR_STA::cli_ID_not_exist_ERROR);
            ReleaseSemaphore(p->bufferMutex, 1, NULL);
            ReleaseSemaphore(p->sendSync, 1, NULL);
            continue;
        }

        // ����Ŀ�Ŀͻ��˵� ID �õ�Ŀ�Ŀͻ��˵� socket
        SOCKET dstSock = p->cliSockMap[dstID];
        std::cout << "dstSock: " << dstSock << std::endl;

        if (p->cliSockSyncMap.find(dstSock) != p->cliSockSyncMap.end())
        {
            // ����ͻ��� socket ��ͬ���ź����Ѿ�����
            // ���ź�������Ч�������Ŀ�Ŀͻ��˷�����Ϣ���߳���Ϊ������״̬
            ReleaseSemaphore(p->bufferMutex, 1, NULL);
            // signal (bufferMutex)
            ReleaseSemaphore(p->cliSockSyncMap[dstSock], 1, NULL);
        }
        else
        {
            // ����ͻ��� socket ��ͬ���ź�����û�д��������������ģ�飬�������η���
            p->error(SocketServer::ERR_STA::no_socket_ERROR);
            ReleaseSemaphore(p->sendSync, 1, NULL);
            ReleaseSemaphore(p->bufferMutex, 1, NULL);
        }
    }
}

DWORD WINAPI SocketServer::CliSendMessageThread(LPVOID IpParameter)
{
    // ���������תΪԭ���Ľṹ������
    Para* para = (Para*)IpParameter;

    while (1)
    {
        // kill ��־Ϊ�棬��ֱ���˳�ѭ���������߳�
        if (para->p->killThrd[para->sock])
            break;

        // ����ר�õķ�����Ϣ�߳�Ĭ�϶���������״̬���� manageSend �߳����������Ϊ������״̬
        if (para->p->cliSockSyncMap.find(para->sock) != para->p->cliSockSyncMap.end())
            WaitForSingleObject(para->p->cliSockSyncMap[para->sock], INFINITE);
        else
            continue;

        std::string msg;
        std::getline(std::cin, msg);

        // wait (bufferMutex)
        WaitForSingleObject(para->p->bufferMutex, INFINITE);

        // ��ָ���Ŀͻ��˷�����Ϣ
        int ret = para->p->Send(para->sock, msg);
        if (ret < 0)
            std::cout << "send error;" << std::endl;
        else
            std::cout << "I say: " << msg << std::endl;

        // signal (bufferMutex)
        ReleaseSemaphore(para->p->bufferMutex, 1, NULL);

        // ��ǰ����������ɣ����� manageSend �̵߳�����һ�η�������
        ReleaseSemaphore(para->p->sendSync, 1, NULL);
    }
    return 0;
}


DWORD WINAPI SocketServer::ReceiveMessageThread(LPVOID IpParameter)
{
    // ���������תΪԭ���Ľṹ������
    Para* para = (Para*)IpParameter;

    while (1)
    {
        std::string recvMsg;
        int ret = para->p->Recv(para->sock, recvMsg);

        // wait ��bufferMutex��
        WaitForSingleObject(para->p->bufferMutex, INFINITE);

        // �ͻ����˳����رոÿͻ��˵��׽��֣��ͷ���Ӧ��Դ
        if (recvMsg == "quit" || ret < 0)
        {
            // ɾ�� socket ��Ӧ��ͬ���ź���, �ͷ� socket ��ͬ���źŵ�ӳ��
            // ע������˳���ܻ�
            CloseHandle(para->p->cliSockSyncMap[para->sock]);
            para->p->cliSockSyncMap.erase(para->sock);

            // �ͷ� ID �� socket ��ӳ��
            for (IdSockMap::iterator iter = para->p->cliSockMap.begin(); iter != para->p->cliSockMap.end(); iter++)
            {
                if (iter->second == para->sock)
                {
                    para->p->cliSockMap.erase(iter);
                    break;
                }
            }

            // �رտͻ��� socket
            closesocket(para->sock);
            std::cout << std::endl << "Attention: A Client has leave..." << std::endl;

            // signal (bufferMutex)
            ReleaseSemaphore(para->p->bufferMutex, 1, NULL);

            // �ͻ����˳������Ӧ���߳�Ҳ����
            // ����������Ϣ�߳�
            para->p->killThrd[para->sock] = true;
            // ��Ҫ���ǽ������߳�ѭ����
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
    // ���ϵȴ��ͻ�������ĵ���
    while (true)
    {
        // �յ�һ���ͻ��˵�����
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

        // ��ͻ���ѯ�� ID
        Send(sockConn, "���������� ID��");
        ClientSockID cliID;
        Recv(sockConn, cliID);

        // �ж� ID �ĸ�ʽ�Ƿ���ȷ


        if (cliSockMap.find(cliID) != cliSockMap.end())
        {
            // ID �Ѿ����ڣ��������ر����ӡ��ͷ���Դ
            error(SocketServer::ERR_STA::cli_ID_duplicated_ERROR);
            closesocket(sockConn);
            ReleaseSemaphore(bufferMutex, 1, NULL);
            continue;
        }
        else
        {
            ReleaseSemaphore(bufferMutex, 1, NULL);
            // �����ͻ��� ID ���� socket ֮���һһӳ��
            cliSockMap.insert(IdSockPair(cliID, sockConn));
        }


        // ���� socket �ͷ����߳�ͬ���ź���֮���һһӳ��
        HANDLE sendSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
        cliSockSyncMap.insert(SockHandlePair(sockConn, sendSemaphore));

        // ���� socket �͹����̹߳رյı�־֮���һһӳ��
        killThrd.insert(SockFlagPair(sockConn, false));

        // �����̲߳����Ľṹ��
        Para* para = new Para;
        para->p = (SocketServer*)this;
        para->sock = sockConn;

        // �����̣߳��������ոÿͻ��˵���Ϣ
        HANDLE receiveThread = CreateThread(NULL, 0, ReceiveMessageThread, (LPVOID)para, 0, NULL);

        // �����̣߳�������ÿͻ��˷�����Ϣ
        HANDLE specSendThread = CreateThread(NULL, 0, CliSendMessageThread, (LPVOID)para, 0, NULL);

        // wait (bufferMutex)
        WaitForSingleObject(bufferMutex, INFINITE);
        if (NULL == receiveThread || NULL == specSendThread)
        {
            // �̴߳���ʧ�ܣ��������ر�����
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
            // �̴߳����ɹ�
            std::cout << std::endl << "Create Receive Client Thread OK." << std::endl;
        }

        // signal (bufferMutex)
        ReleaseSemaphore(bufferMutex, 1, NULL);

        // �ͷž��
        CloseHandle(receiveThread);
        CloseHandle(specSendThread);
    }
}

void SocketServer::waitThread()
{
    // �ȴ��߳̽���
    WaitForSingleObject(sendManagerThread, INFINITE);
}
