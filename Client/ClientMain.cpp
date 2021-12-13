#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <Windows.h>
#include "SocketClient.h"

using namespace std;

HANDLE bufferMutex;     // �����ܻ���ɹ�����ͨ�ŵ��ź������
SocketClient myClient;

int main()
{
    //  send(sockClient, "\nAttention: A Client has enter...\n", strlen("Attention: A Client has enter...\n")+1, 0);
    send(myClient.cliSocket, "\nAttention: A Client has enter...\n", 200, 0);

    bufferMutex = CreateSemaphore(NULL, 1, 1, NULL);

    DWORD WINAPI SendMessageThread(LPVOID IpParameter);
    DWORD WINAPI ReceiveMessageThread(LPVOID IpParameter);

    HANDLE sendThread = CreateThread(NULL, 0, SendMessageThread, NULL, 0, NULL);
    HANDLE receiveThread = CreateThread(NULL, 0, ReceiveMessageThread, NULL, 0, NULL);


    WaitForSingleObject(sendThread, INFINITE);  // �ȴ��߳̽���
    CloseHandle(sendThread);
    CloseHandle(receiveThread);
    CloseHandle(bufferMutex);

    printf("End linking...\n");
    printf("\n");
    system("pause");
    return 0;
}


DWORD WINAPI SendMessageThread(LPVOID IpParameter)
{
    while (1) {
        string talk;
        getline(cin, talk);
        WaitForSingleObject(bufferMutex, INFINITE);     // P����Դδ��ռ�ã�
        if ("quit" == talk) {
            talk.push_back('\0');
            //          send(sockClient, talk.c_str(), talk.size(), 0);
            send(myClient.cliSocket, talk.c_str(), 200, 0);
            break;
        }
        else {
            talk.append("\n");
        }
        printf("\nI Say:(\"quit\"to exit):");
        cout << talk;
        //  send(sockClient, talk.c_str(), talk.size(), 0); // ������Ϣ
        send(myClient.cliSocket, talk.c_str(), 200, 0); // ������Ϣ
        ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�
    }
    return 0;
}


DWORD WINAPI ReceiveMessageThread(LPVOID IpParameter)
{
    while (1) {
        char recvBuf[300];
        recv(myClient.cliSocket, recvBuf, 200, 0);
        WaitForSingleObject(bufferMutex, INFINITE);     // P����Դδ��ռ�ã�

        printf("%s Says: %s\n", "Server", recvBuf);     // ������Ϣ

        ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�
    }
    return 0;
}