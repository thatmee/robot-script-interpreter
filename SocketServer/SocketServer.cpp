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
}


void SocketServer::error(SocketServer::ERR_STA err)
{
    dbg.out("error: " + std::string(magic_enum::enum_name(err)) + "\n", logName);
}


void SocketServer::initSocketServer()
{
    int iRet = 0;

    dbg.out("====================== Server ==========================\n", logName);

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
    dbg.out("done! load the socket data.\n", logName);


    // ������ʽ�׽���
    srvSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (srvSocket == INVALID_SOCKET)
    {
        error(SocketServer::ERR_STA::socket_INVALID_SOCKET);
        return;
    }
    dbg.out("done! create srvSocket.\n", logName);

    // ��ʼ����������ַ�����
    srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(6000);
    dbg.out("done! init server address.\n", logName);

    // ��
    iRet = bind(srvSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
    if (iRet == SOCKET_ERROR)
    {
        error(SocketServer::ERR_STA::bind_SOCKET_ERROR);
        return;
    }
    dbg.out("done! bind.\n", logName);

    //����
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


DWORD WINAPI SocketServer::CliSendMessageThread(LPVOID IpParameter)
{
    // ���������תΪԭ���Ľṹ������
    Para* para = (Para*)IpParameter;

    while (1)
    {
        // kill ��־Ϊ�棬��ֱ���˳�ѭ���������߳�
        if (para->p->killThrd[para->sock])
            break;

        std::string msg;
        int ret;

        // ���� scriptSrv ��ָʾִ�в�ͬ������
        ScriptServer::INTERPRET_STA todo = para->p->scriptSrv.srvInterpret(para->id);
        switch (todo)
        {
        case ScriptServer::INTERPRET_STA::Do:
            break;
        case ScriptServer::INTERPRET_STA::Listen:
            // ��Ϊ����״̬
            para->p->listenValid[para->sock] = true;
            WaitForSingleObject(para->p->listenSync[para->sock], INFINITE);
            // ��������Ϣ�󣬹رռ���״̬
            para->p->listenValid[para->sock] = false;
            // ���ò�������״̬
            para->p->scriptSrv.setFinished(para->id);
            break;
        case ScriptServer::INTERPRET_STA::Exit:
            para->p->killThrd[para->sock] = true;
            break;
        case ScriptServer::INTERPRET_STA::Out:
            para->p->scriptSrv.getOutputMsg(para->id, msg);
            // wait (bufferMutex)
            WaitForSingleObject(para->p->bufferMutex, INFINITE);

            // ��ָ���Ŀͻ��˷�����Ϣ
            ret = para->p->Send(para->sock, msg);
            if (ret < 0)
                para->p->dbg.out("send error;\n", para->p->logName);
            else
                para->p->dbg.out("I say: " + msg + "\n", para->p->logName);

            // signal (bufferMutex)
            ReleaseSemaphore(para->p->bufferMutex, 1, NULL);

            // ���ò�������״̬
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
    // ���������תΪԭ���Ľṹ������
    Para* para = (Para*)IpParameter;

    while (1)
    {
        std::string recvMsg;
        int ret = para->p->Recv(para->sock, recvMsg);

        // wait ��bufferMutex��
        WaitForSingleObject(para->p->bufferMutex, INFINITE);

        // �ͻ����˳����رոÿͻ��˵��׽��֣��ͷ���Ӧ��Դ
        if (recvMsg == "quit" || ret < 0 || para->p->killThrd[para->sock])
        {
            // ɾ���û�
            para->p->scriptSrv.deleteUser(para->id);

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
            para->p->dbg.out("Attention: A Client has leave...\n", para->p->logName);
            //std::cout << std::endl << "" << std::endl;


            // signal (bufferMutex)
            ReleaseSemaphore(para->p->bufferMutex, 1, NULL);

            // �ͻ����˳������Ӧ���߳�Ҳ����
            // ����������Ϣ�߳�
            para->p->killThrd[para->sock] = true;
            // ��Ҫ���ǽ������߳�ѭ����
            break;
        }

        // ���������� listen ��ʱ�򣬽����յ�����Ϣ���浽 user ��
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

        if (cliSockMap.find(cliID) != cliSockMap.end())
        {
            // ID �Ѿ����ڣ��������ر����ӡ��ͷ���Դ
            error(SocketServer::ERR_STA::cli_ID_duplicated_ERROR);
            closesocket(sockConn);
            ReleaseSemaphore(bufferMutex, 1, NULL);
            continue;
        }

        // ����һ���û�
        if (scriptSrv.createUser(cliID) != ScriptServer::NEW_USER_STA::Succeed)
        {
            // �������ɹ����ر����ӡ��ͷ���Դ
            closesocket(sockConn);
            ReleaseSemaphore(bufferMutex, 1, NULL);
            continue;
        }
        else
        {
            // �����ͻ��� ID ���� socket ֮���һһӳ��
            cliSockMap.insert(IdSockPair(cliID, sockConn));
        }


        // ���� socket �ͷ����߳�ͬ���ź���֮���һһӳ��
        HANDLE sendSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
        cliSockSyncMap.insert(SockHandlePair(sockConn, sendSemaphore));

        // ���� socket ������ listen ���ź���֮���һһӳ��
        HANDLE listenSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
        listenSync.insert(SockHandlePair(sockConn, listenSemaphore));

        // ���� socket �ͱ�־ listen �Ƿ���Ч����֮���һһӳ��
        listenValid.insert(SockFlagPair(sockConn, false));

        // ���� socket �͹����̹߳رյı�־֮���һһӳ��
        killThrd.insert(SockFlagPair(sockConn, false));

        // �����̲߳����Ľṹ��
        Para* para = new Para;
        para->p = (SocketServer*)this;
        para->sock = sockConn;
        para->id = cliID;

        // �����̣߳��������ոÿͻ��˵���Ϣ
        HANDLE receiveThread = CreateThread(NULL, 0, ReceiveMessageThread, (LPVOID)para, 0, NULL);

        // �����̣߳�������ÿͻ��˷�����Ϣ
        HANDLE specSendThread = CreateThread(NULL, 0, CliSendMessageThread, (LPVOID)para, 0, NULL);

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
            dbg.out("Create Receive Client Thread OK.\n", logName);
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
    WaitForSingleObject(allKill, INFINITE);
}
