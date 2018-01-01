#include <Winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
using namespace std;
#include "DataStructure.cpp"
#include "virtualPacket.cpp"
#include "RouteTableLS.cpp"
#define PORT 8080
#define localname 'A'

class controller
{
public:
	char localaddr[SIZE];          // 本机地址
    char name;
	int port;                      // 端口号
	std::vector<route> routelist;  // 邻居路由表
	RouteTableLS table;

	SOCKET sock;                  // socket模块
	sockaddr_in sockAddr;         // 绑定的socket地址

	controller(char *localaddr, char name, int port, vector<route> routelist);


	void listen();   // 绑定socket
	void run();      // start监听
	void sendHeartBeatPacket();   // 向其他所有路由器发送心跳监测报文，监测邻居是否被down掉
	void sendUpdatePacket();      // 发送路由表信息
	void sendNormalPacket(char *dstip, char *message);
    void sendResponsePacket(char *dstip, char *RESPONSE);  // 发送回响信息，表明是否收到了这个报文
	void handleReceivedPacket(char *recvBuf);          // 处理接收的报文 根据packet类型调用以下四种处理方法
	void handleHeartBeatPacket(virtualPacket packet);  // 处理心跳检测报文
	void handleUpdatePacket(virtualPacket packet);     // 处理路由表信息更新报文
	void handleNormalPacket(virtualPacket packet);     // 处理普通报文
	void forward(virtualPacket packet);                // 转发报文
	void handleResponsePacket(virtualPacket packet);   // 处理响应报文

	SOCKADDR_IN sendPacket(char *sendMessage, char *dst);

	~controller();
};
