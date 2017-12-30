#include <Winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include "routeInfo.cpp"
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
    std::vector<route> routerlist;  // 邻居路由表
    RouteTableDV table;

    SOCKET sock;              // socket模块
    sockaddr_in sockAddr;         // 绑定的socket地址

    controller(char *localaddr, int port, vector<route> routelist) {
        strcpy(this->localaddr,localaddr);
        this->port = port;
        this->routelist = routelist;
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
    }

    // 绑定socket
    void listen() {
        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_addr.s_addr = inet_addr(localaddr);
        sockAddr.sin_port = htons(port);
        bind(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
        cout << "Listen at PORT " << port << endl;
    }

    // start监听
    void run() {
        while (1) {
            listen(sock, 20);

            int nSize = sizeof(sockAddr);

            char recvBuf[MAXBYTE];
            recv(sock, recvBuf, MAXBYTE, 0, (SOCKADDR*)&sockAddr, &nSize);
            handleReceivedPacket(recvBuf);
            // getPack or forward packet
        }
    }

    // 向其他所有路由器发送心跳监测包，监测邻居是否被down掉
    void sendHeartBeatPacket() {
        for (auto r : routerlist) {
            char sendMessage[MAXBYTE];
            Addr local('A', localaddr);
            Addr dst(r.addr.name, r.addr.ipaddress);
            virtualPacket heartbeatPacket(2, local, dst, NULL);
            heartbeatPacket.constructHeartBeatPacket(sendMessage);
            sendPacket(sendMessage, dst.ipaddress);
            char recvBuf[MAXBYTE];
            SOCKADDR_IN recvSock;
            int len = sizeof(recvSock);
            recvfrom(sock, recvBuf, MAXBYTE, 0, (SOCKADDR*)&recvSock, &len);
            if (strcmp(recvBuf, "I am alive!") == 0)
                continue;
            else {
                r.setDumped();
                // update routeList
            }
        }
    }

    // 发送路由表信息
    void sendUpdatePacket() {
        for (auto r : routerlist) {
            char sendMessage[MAXBYTE];
            Addr local('A', localaddr);
            Addr dst(r.addr.name, r.addr.ipaddress);
            virtualPacket updatePacket(1, local, dst, NULL);
            updatePacket.constructRouterInfoPacket(sendMessage, table.routetable);
            sendPacket(sendMessage,dst.ipaddress);
        }
    }

    // 发送普通的packet信息，字符串呀乱七八糟的
    void sendNormalPacket(char *dst, char *message) {
        char sendMessage[MAXBYTE];
        Addr local('A', localaddr);
        Addr dst(r.addr.name, r.addr.ipaddress);
        virtualPacket normalPacket(0, local, dst, NULL);
        normalPacket.constructNormalPacket(sendMessage, message);
        sendPacket(sendMessage, dst.ipaddress);
    }

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

    void sendPacket(char *sendMessage, char *dst) {
        SOCKADDR_IN addr_Server, addr_Clt; //服务器的地址等信息
        addr_Server.sin_family = AF_INET;
        addr_Server.sin_port = htons(PORT);
        addr_Server.sin_addr.S_un.S_addr= dst;

        sendto(sock, sendMessage, strlen(sendMessage), 0, (SOCKADDR*)&addr_Clt, sizeof(SOCKADDR));
    }

    ~controller() {
        closesocket(sock);
        WSACleanup();
        cout << "Server Socketreleased" << endl;
    }
};