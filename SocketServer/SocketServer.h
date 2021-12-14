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

using ClientSockID = std::string;
using IdSockMap = std::unordered_map<ClientSockID, SOCKET>;
using IdSockPair = std::pair<ClientSockID, SOCKET>;
using SockSyncMap = std::unordered_map<SOCKET, HANDLE>;
using SockSyncPair = std::pair<SOCKET, HANDLE>;

class SocketServer
{
private:
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

    int connectNumMax = 10;

    ///// @brief ��ʼ�� socket �Լ������߳�
    //void init();

    /// @brief ��ʼ�� socket ��������
    void initSocketServer();

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

    /// @brief ��������
    /// @param err ����״̬
    void error(SocketServer::ERR_STA err);

    DWORD static WINAPI CliSendMessageThread(LPVOID IpParameter);
    DWORD static WINAPI ReceiveMessageThread(LPVOID IpParameter);
    DWORD static WINAPI ManageSendThread(LPVOID IpParameter);

public:

    /// @brief �׽��ֿ�
    WSADATA wsaData;

    /// @brief �������������������׽���
    SOCKET srvSocket;

    /*/// @brief ��Ӧ�ͻ������ӵ��׽���
    SOCKET accSocket;*/


    /// @brief ���ӵĿͻ��˵�ַ��
    SOCKADDR_IN clientAddr;

    /// @brief ��������ַ��
    SOCKADDR_IN srvAddr;

    /// @brief �����ܻ���ɹ�����ͨ�ŵ��ź������
    HANDLE bufferMutex;

    HANDLE sendSync;

    /// @brief ���ڹ������з�����Ϣ������
    HANDLE sendManagerThread;

    /// @brief ���ӵ������пͻ����׽���
    //std::vector<SOCKET> clientSocketGroup;
    IdSockMap cliSockMap;


    /// @brief ��Բ�ͬ������Ϣ�̵߳�ͬ���ź���
    SockSyncMap cliSockSyncMap;

    //std::vector<HANDLE> cliSendThreads;
    //std::vector<HANDLE> recvThreads;

    SocketServer(int connectNumMax_);

    ~SocketServer();

    /// @brief �ȴ��������˵�����
    //void Accept();

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

struct Para {
    SocketServer* p;
    SOCKET sock;
};

