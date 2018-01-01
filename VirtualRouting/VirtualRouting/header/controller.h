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
	char localaddr[SIZE];         // 本机地址
    char name;
	int port;                     // 端口号
	std::vector<route> routelist;  // 邻居路由表
	RouteTableLS table;

	SOCKET sock;              // socket模块
	sockaddr_in sockAddr;         // 绑定的socket地址

	controller(char *localaddr, char name, int port, vector<route> routelist);

	// 绑定socket
	void listen();

	// start监听
	void run();

	// 向其他所有路由器发送心跳监测包，监测邻居是否被down掉
	void sendHeartBeatPacket();
	// 发送路由表信息
	void sendUpdatePacket();

	// 发送普通的packet信息，字符串呀乱七八糟的
	void sendNormalPacket(char *dstip, char *message);

	// 发送回响信息，表明是否收到了这个包
	void sendResponsePacket(char *dstip, char *RESPONSE);

	// 处理接收的包，根据packet类型调用以下四种处理方式
	void handleReceivedPacket(char *recvBuf);

	// 处理心跳检测包
	void handleHeartBeatPacket(virtualPacket packet);

	// 处理路由表信息更新包
	void handleUpdatePacket(virtualPacket packet);
	// 处理普通的包
	void handleNormalPacket(virtualPacket packet);

	// 转发普通的包
	void forward(virtualPacket packet);

	// 处理响应包
	void handleResponsePacket(virtualPacket packet);

	SOCKADDR_IN sendPacket(char *sendMessage, char *dst);

	~controller();
};
