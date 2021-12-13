#pragma once
#include <iostream>
#include <string>
#include <lib\magic_enum.hpp>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")


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
        accept_SOCKET_ERROR
    };

    int connectNumMax = 10;

    /// @brief ��ʼ�� socket ��������
    void initSocketServer();

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

    /// @brief ��������
    /// @param err ����״̬
    void error(SocketServer::ERR_STA err);
public:

    /// @brief �׽��ֿ�
    WSADATA wsaData;

    /// @brief �������������������׽���
    SOCKET srvSocket;

    /// @brief ��Ӧ�ͻ������ӵ��׽���
    SOCKET accSocket;

    /// @brief ���ӵĿͻ��˵�ַ��
    SOCKADDR_IN clientAddr;

    /// @brief ��������ַ��
    SOCKADDR_IN srvAddr;

    SocketServer(int connectNumMax_);

    ~SocketServer();

    /// @brief �ȴ��������˵�����
    void Accept();

    /// @brief ��Զ˷�����Ϣ
    /// @param msg Ҫ���͵���Ϣ�ַ���
    /// @return -1 ��ʾ����ʧ�ܣ�����Ϊ�ɹ����͵��ַ���
    int Send(std::string msg);

    /// @brief ���նԶ���Ϣ
    /// @param msg ������յ�����Ϣ
    /// @return -1 ��ʾ����ʧ�ܣ�����Ϊ�ɹ����յ��ַ���
    int Recv(std::string& msg);
};

