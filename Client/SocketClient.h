#pragma once
#include <iostream>
#include <string>
#include <lib\magic_enum.hpp>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

class SocketClient
{
private:
    enum class ERR_STA
    {
        WSAStartup_ReturnNoneZero,
        WSADATA_versionWrong,
        socket_INVALID_SOCKET,
        connect_ERROR
    };

    int connectNumMax = 10;

    /// @brief ��������
    /// @param err ����״̬
    void error(SocketClient::ERR_STA err);

    /// @brief ��ʼ���ͻ��� socket
    void initSocketClient();

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

public:

    /// @brief �׽��ֿ�
    WSADATA wsaData;

    /// @brief �ͻ��˵��׽���
    SOCKET cliSocket;

    /// @brief ��������ַ��
    SOCKADDR_IN srvAddr;

    SocketClient();
    ~SocketClient();

    /// @brief ��Զ˷�����Ϣ
    /// @param msg Ҫ���͵���Ϣ�ַ���
    /// @return -1 ��ʾ����ʧ�ܣ�����Ϊ�ɹ����͵��ַ���
    int Send(std::string msg);

    /// @brief ���նԶ���Ϣ
    /// @param msg ������յ�����Ϣ
    /// @return -1 ��ʾ����ʧ�ܣ�����Ϊ�ɹ����յ��ַ���
    int Recv(std::string& msg);
};

