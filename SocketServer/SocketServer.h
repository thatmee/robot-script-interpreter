#pragma once
#include <cstdio>
#include <iostream>
#include <cstring>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <lib\magic_enum.hpp>
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")
#include "Interpreter/ScriptServer.h"

using ClientSockID = std::string;
using IdSockMap = std::unordered_map<ClientSockID, SOCKET>;
using IdSockPair = std::pair<ClientSockID, SOCKET>;
using SockHandleMap = std::unordered_map<SOCKET, HANDLE>;
using SockHandlePair = std::pair<SOCKET, HANDLE>;
using SockFlagMap = std::unordered_map<SOCKET, bool>;
using SockFlagPair = std::pair<SOCKET, bool>;

class SocketServer
{
private:
    /************************** ������ ***********************************/

    /// @brief ���д���״̬
    enum class ERR_STA
    {
        WSAStartup_ReturnNoneZero,
        WSADATA_versionWrong,
        socket_INVALID_SOCKET,
        bind_SOCKET_ERROR,
        listen_SOCKET_ERROR,
        accept_SOCKET_ERROR,
        create_thread_ERROR,
        cli_ID_duplicated_ERROR,
        cli_ID_not_exist_ERROR,
        no_socket_ERROR
    };

    /// @brief ��������
    /// @param err ����״̬
    void error(SocketServer::ERR_STA err);


    /*********************** socket ��ʼ�� **********************************/

    /// @brief �׽��ֿ�
    WSADATA wsaData;

    /// @brief �������������������׽���
    SOCKET srvSocket;

    /// @brief ���ӵĿͻ��˵�ַ��
    SOCKADDR_IN clientAddr;

    /// @brief ��������ַ��
    SOCKADDR_IN srvAddr;

    /// @brief Ĭ�����������Ϊ 10
    int connectNumMax = 10;

    /// @brief ��ʼ�� socket ��������
    void initSocketServer();


    /******************** socket ��д�����ķ�װ *******************************/

    /// @brief �ӶԶ˶�ȡָ���������ַ�
    /// @param buf �����ȡ�����ַ�
    /// @param size ��ȡ������
    /// @return -1 ��ʾ��ȡʧ�ܣ�����Ϊ�ɹ���ȡ���ַ���
    int readn(SOCKET srcSocket, char* buf, int size);

    /// @brief ��Զ˷���ָ���������ַ�
    /// @param msg Ҫ���͵�����
    /// @param size Ҫ���͵��ַ�����
    /// @return -1 ��ʾ����ʧ�ܣ�����Ϊ�ɹ����͵��ַ���
    int writen(SOCKET dstSocket, const char* msg, int size);


    /*********************** C/S ͨ�� ***************************************/

    /// @brief ���ӵ������пͻ����׽���
    IdSockMap cliSockMap;

    /// @brief ��Բ�ͬ������Ϣ�̵߳�ͬ���ź���
    SockHandleMap cliSockSyncMap;

    /// @brief ���ڰ�ȫ�˳��̵߳ı�־
    SockFlagMap killThrd;

    /// @brief �����ܻ���ɹ�����ͨ�ŵ��ź������
    HANDLE bufferMutex;

    /// @brief �ṩ sendManager �� cliSend �߳�֮��ͬ�����ź���
    HANDLE sendSync;

    /// @brief ���ڹ������з���������߳̾��
    //HANDLE sendManagerThread;

    /// @brief ���������������Ĺر�
    HANDLE allKill;

    /// @brief �������з���������߳�
    /// @param IpParameter this ָ��
    DWORD static WINAPI ManageSendThread(LPVOID IpParameter);

    /// @brief ��Ե����ͻ��˷�����Ϣ���߳�
    /// @param IpParameter �ṹ�� Para { p = this; sock = �ͻ��˶�Ӧ socket; }
    DWORD static WINAPI CliSendMessageThread(LPVOID IpParameter);

    /// @brief ��Ե����ͻ��˽�����Ϣ���߳�
    /// @param IpParameter �ṹ�� Para { p = this; sock = �ͻ��˶�Ӧ socket; }
    DWORD static WINAPI ReceiveMessageThread(LPVOID IpParameter);


    /*********************** script interpreter ******************************/

    DBG dbg;
    ScriptServer scriptSrv;
    SockHandleMap listenSync;
    SockFlagMap listenValid;

public:
    SocketServer(int connectNumMax_);

    ~SocketServer();

    /// @brief ��Զ˷�����Ϣ
    /// @param msg Ҫ���͵���Ϣ�ַ���
    /// @return -1 ��ʾ����ʧ�ܣ�����Ϊ�ɹ����͵��ַ���
    int Send(SOCKET dstSocket, std::string msg);

    /// @brief ���նԶ���Ϣ
    /// @param msg ������յ�����Ϣ
    /// @return -1 ��ʾ����ʧ�ܣ�����Ϊ�ɹ����յ��ַ���
    int Recv(SOCKET srcSocket, std::string& msg);

    /// @brief ѭ�����տͻ�����������
    void Accept();

    /// @brief �ȴ��߳̽���
    void waitThread();
};

/// @brief �������̴߳��ݲ����Ľṹ��
struct Para {
    SocketServer* p;
    SOCKET sock;
    ClientSockID id;
};

