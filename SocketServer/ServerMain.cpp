#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <vector>
#include <iterator>
#include <algorithm>
#include <Winsock2.h>
#include <Windows.h>
#include "SocketServer.h"

using namespace std;

HANDLE bufferMutex;     // �����ܻ���ɹ�����ͨ�ŵ��ź������
SOCKET sockConn;        // �ͻ��˵��׽���
vector <SOCKET> clientSocketGroup;
SocketServer myServer(20);

int main()
{

    bufferMutex = CreateSemaphore(NULL, 1, 1, NULL);

    DWORD WINAPI SendMessageThread(LPVOID IpParameter);
    DWORD WINAPI ReceiveMessageThread(LPVOID IpParameter);

    HANDLE sendThread = CreateThread(NULL, 0, SendMessageThread, NULL, 0, NULL);

    while (true) {    // ���ϵȴ��ͻ�������ĵ���
        sockConn = accept(myServer.srvSocket, NULL, NULL);
        if (SOCKET_ERROR != sockConn) {
            clientSocketGroup.push_back(sockConn);
        }
        HANDLE receiveThread = CreateThread(NULL, 0, ReceiveMessageThread, (LPVOID)sockConn, 0, NULL);
        WaitForSingleObject(bufferMutex, INFINITE);     // P����Դδ��ռ�ã�
        if (NULL == receiveThread) {
            printf("\nCreatThread AnswerThread() failed.\n");
        }
        else {
            printf("\nCreate Receive Client Thread OK.\n");
        }
        ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�
    }

    WaitForSingleObject(sendThread, INFINITE);  // �ȴ��߳̽���
    CloseHandle(sendThread);
    CloseHandle(bufferMutex);
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
    /*  if("quit" == talk){
            ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�
            return 0;
        }
        else*/
        {
            talk.append("\n");
        }
        printf("I Say:(\"quit\"to exit):");
        cout << talk;
        for (int i = 0; i < clientSocketGroup.size(); ++i) {
            //      send(clientSocketGroup[i], talk.c_str(), talk.size(), 0);   // ������Ϣ
            send(clientSocketGroup[i], talk.c_str(), 200, 0);   // ������Ϣ
        }
        ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�
    }
    return 0;
}


DWORD WINAPI ReceiveMessageThread(LPVOID IpParameter)
{
    SOCKET ClientSocket = (SOCKET)(LPVOID)IpParameter;
    while (1) {
        char recvBuf[300];
        recv(ClientSocket, recvBuf, 200, 0);
        WaitForSingleObject(bufferMutex, INFINITE);     // P����Դδ��ռ�ã�

        if (recvBuf[0] == 'q' && recvBuf[1] == 'u' && recvBuf[2] == 'i' && recvBuf[3] == 't' && recvBuf[4] == '\0') {
            vector<SOCKET>::iterator result = find(clientSocketGroup.begin(), clientSocketGroup.end(), ClientSocket);
            clientSocketGroup.erase(result);
            closesocket(ClientSocket);
            ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�
            printf("\nAttention: A Client has leave...\n", 200, 0);
            break;
        }

        printf("%s Says: %s\n", "One Client", recvBuf);     // ������Ϣ

        ReleaseSemaphore(bufferMutex, 1, NULL);     // V����Դռ����ϣ�
    }
    return 0;
}