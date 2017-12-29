#include <Winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include "DataSreuctrue.cpp"
#define PORT 8080
#define localaddr "192.168.1.3"
#define response "I have Received your data"
using namespace std;

// controller
// 接收packet，如果刚是给自己的，解析，否则转发
// 每隔两秒发送一次路由信息，以及侦测包，查看自己的邻居是否还在活动（涉及down掉的情况）
// 发送packet，比如确认收到等信息
//



class controller
{
public:
    char localaddr[SIZE];         // 本机地址
    int port;                     // 端口号
    std::vector<route> neighbor;  // 邻居路由表

    SOCKET servSock;              // socket模块
    sockaddr_in sockAddr;         // 绑定的socket地址

    controller(localaddr, port, routelist) {
        this->localaddr = localaddr;
        this->port = port;
        this->routelist = routelist;
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
    }

    // 绑定socket
    void listen() {
        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_addr.s_addr = inet_addr(localaddr);
        sockAddr.sin_port = htons(port);
        bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
        cout << "Listen at PORT " << port << endl;
    }

    // start监听
    void run() {
        while (1) {
            listen(servSock, 20);

            int nSize = sizeof(sockAddr);

            char recvBuf[MAXBYTE];
            recv(servSock, recvBuf, MAXBYTE, 0, (SOCKADDR*)&sockAddr, &nSize);
            handleReceivedPacket(recvBuf);
            // getPack or forward packet
        }
    }

    // 向邻居发送心跳监测包，监测邻居是否被down掉
    void sendHearBeatPacket();

    // 发送路由表信息
    void sendUpdatePacket();

    // 发送普通的packet信息，字符串呀乱七八糟的
    void sendNormalPacket();

    // 发送回响信息，表明是否收到了这个包
    void sendResponsePacket();

    // 处理接收的包，根据packet类型调用以下三种处理方式
    void handleRececivedPacket(virtualPacket packet);

    // 处理心跳检测包
    void handleHeartBeatPacket(virtualPacket packet);

    // 处理路由表信息更新包
    void handleUpdatePacket(virtualPacket packet);

    // 处理普通的包
    void handleNormalPacket(virtualPacket packet);

    // 转发报
    void forward(virtualPacket packet);

    ~controller() {
        closesocket(servSock);
        WSACleanup();
        cout << "Server Socketreleased" << endl;
    }
};