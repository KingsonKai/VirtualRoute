#include <Winsock2.h>
#include <cstdio>
#include <iostream>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
int main()
{
    WSADATA wsaData;
    SOCKET sockClient;//客户端Socket
    SOCKADDR_IN addrServer;//服务端地址
    WSAStartup(MAKEWORD(2,2),&wsaData);
    //新建客户端socket
    sockClient=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    //定义要连接的服务端地址
    addrServer.sin_addr.S_un.S_addr=inet_addr("192.168.191.001");//目标IP(127.0.0.1是回送地址)
    addrServer.sin_family=AF_INET;
    addrServer.sin_port=htons(8080);//连接端口6000
    int len = sizeof(addrServer);
    //连接到服务端

    //发送数据
    char message[1000]="0127.000.000.001*127.000.000.001*TEST";
    char recvBuf[100];
    sendto(sockClient, message, strlen(message), 0, (SOCKADDR*)&addrServer, len);
    int ret = recvfrom(sockClient, recvBuf, 100, 0, (SOCKADDR*)&addrServer, &len);
    recvBuf[ret] = '\0';
    cout << recvBuf << endl;

    closesocket(sockClient);
    WSACleanup();
    system("pause");
}
