#pragma once
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <lib\magic_enum.hpp>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")



class SocketClient
{
private:
    /************************** ������ ***********************************/

    /// @brief ���д���״̬
    enum class ERR_STA
    {
        WSAStartup_ReturnNoneZero,
        WSADATA_versionWrong,
        socket_INVALID_SOCKET,
        connect_ERROR
    };

    /// @brief ��������
    /// @param err ����״̬
    void error(SocketClient::ERR_STA err);


    /*********************** socket ��ʼ�� **********************************/

    /// @brief �׽��ֿ�
    WSADATA wsaData;

    /// @brief �ͻ��˵��׽���
    SOCKET cliSocket;

    /// @brief ��������ַ��
    SOCKADDR_IN srvAddr;

    /// @brief ��ʼ���ͻ��� socket
    void initSocketClient();


    /******************** socket ��д�����ķ�װ *******************************/

    /// @brief �ӶԶ˶�ȡָ���������ַ�
    /// @param buf �����ȡ�����ַ�
    /// @param size ��ȡ������
    /// @return -1 ��ʾ��ȡʧ�ܣ�����Ϊ�ɹ���ȡ���ַ���
    int readn(char* buf, int size);

    /// @brief ��Զ˷���ָ���������ַ�
    /// @param msg Ҫ���͵�����
    /// @param size Ҫ���͵��ַ�����
    /// @return -1 ��ʾ����ʧ�ܣ�����Ϊ�ɹ����͵��ַ���
    int writen(const char* msg, int size);


    /*********************** C/S ͨ�� ***************************************/

    /// @brief �����ܻ���ɹ�����ͨ�ŵ��ź������
    HANDLE bufferMutex;

    /// @brief ������Ϣ���߳�
    HANDLE sendThread;

    /// @brief ������Ϣ���߳�
    HANDLE receiveThread;

    /// @brief ���ڰ�ȫ�˳��̵߳ı�־
    bool killThrd;

    /// @brief ��ǰ�ͻ������ڷ�����Ϣ���߳�
    /// @param IpParameter this ָ��
    DWORD static WINAPI SendMessageThread(LPVOID IpParameter);

    /// @brief ��ǰ�ͻ������ڽ�����Ϣ���߳�
    /// @param IpParameter this ָ��
    DWORD static WINAPI ReceiveMessageThread(LPVOID IpParameter);


public:

    SocketClient();

    ~SocketClient();

    /// @brief �ȴ��߳̽���
    void waitThread();

    /// @brief ��Զ˷�����Ϣ
    /// @param msg Ҫ���͵���Ϣ�ַ���
    /// @return -1 ��ʾ����ʧ�ܣ�����Ϊ�ɹ����͵��ַ���
    int Send(std::string msg);

    /// @brief ���նԶ���Ϣ
    /// @param msg ������յ�����Ϣ
    /// @return -1 ��ʾ����ʧ�ܣ�����Ϊ�ɹ����յ��ַ���
    int Recv(std::string& msg);
};

