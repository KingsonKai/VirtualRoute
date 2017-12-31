#include <Winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include "routeInfo.cpp"
#define PORT 8080
#define localaddr "192.168.1.3"
#define RESPONSE "I have Received your data"
#define MAXBYTE 10000
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
		strcpy(this->localaddr, localaddr);
		this->port = port;
		this->routelist = routelist;
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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
			int nSize = sizeof(sockAddr);

			char recvBuf[MAXBYTE];
			int ret = recvfrom(sock, recvBuf, MAXBYTE, 0, (SOCKADDR*)&sockAddr, &nSize);
			if (ret > 0) {
				handleReceivedPacket(recvBuf);
			}
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
			sendPacket(sendMessage, dst); //directly
			char recvBuf[MAXBYTE];
			recvfrom(sock, recvBuf, MAXBYTE, 0, (SOCKADDR*)&recvSock, &len);
			virtualPacket packet;
			packet.makePacket(recvBuf);
			if (strcmp(packet.getMessage(), "I am alive!") == 0)
				continue;
			else {
				r.setDumped();
				// update routeList
				// send updatepacket
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
			sendPacket(sendMessage, table.getNextHop(dst));
		}
	}

	// 发送普通的packet信息，字符串呀乱七八糟的
	void sendNormalPacket(char *dstip, char *message) {
		char sendMessage[MAXBYTE];
		Addr local('A', localaddr);
		Addr dst('1', dstip);
		virtualPacket normalPacket(0, local, dst, NULL);
		normalPacket.constructNormalPacket(sendMessage, message);
		sendPacket(sendMessage, table.getNextHop(dst));
	}

	// 发送回响信息，表明是否收到了这个包
	void sendResponsePacket(char *dstip) {
		char sendMessage[MAXBYTE];
		Addr local('A', localaddr);
		Addr dst('1', dstip);
		virtualPacket responsePacket(3, local, dst, NULL);
		char *message;
		strcpy(message, RESPONSE);
		responsePacket.constructResponsePacket(sendMessage, message);
		/* 将包发送至下一跳路由器 */
		sendPacket(sendMessage, table.getNextHop(dst));
	}

	// 处理接收的包，根据packet类型调用以下四种处理方式
	void handleReceivedPacket(char *recvBuf) {
		/* 将收到的字符串数据转化为数据包格式 */
		virtualPacket packet;
		packet.makePacket(recvBuf);
		if (packet.type == 0) {
			handleNormalPacket(packet);
		}
		else if (packet.type == 1) {
			handleUpdatePacket(packet);
		}
		else if (packet.type == 2) {
			handleHeartBeatPacket(packet);
		}
		else if (packet.type == 3) {
			handleResponsePacket(packet);
		}
	}

	// 处理心跳检测包
	void handleHeartBeatPacket(virtualPacket packet) {
		// 若有收到，回应主机，目前正在工作
		char dstip[20];
		char source[20];
		strcpy(dstip, packet.getDst().ipaddress);
		strcpy(source, packet.getSource().ipaddress);
		if (strcmp(dstip, localaddr) == 0) {
			virtualPacket packet(2, localaddr, source, NULL);
			char sendMessage[MAXBYTE];
			packet.constructHeartBeatPacket(sendMessage);
			sendPacket(sendMessage, source);
		}
	}

	// 处理路由表信息更新包
	void handleUpdatePacket(virtualPacket packet) {
		// 1. 解析数据包提取信息
		// 2. 根据提取的信息决定是否要调用算法更新路由表
		// 3. 如果更新了路由表，要发给邻居一份路由信息
	}

	// 处理普通的包
	void handleNormalPacket(virtualPacket packet) {
		char dstip[20];
		strcpy(dstip, packet.getDst().ipaddress);
		if (strcmp(dstip, localaddr) == 0) {
			packet.print();
			sendResponsePacket(dstip);
		}
		else {
			forwardNormalPacket(packet);
		}
	}

	// 转发普通的包
	void forwardNormalPacket(virtualPacket packet) {
		char sendMessage[MAXBYTE];
		packet.constructNormalPacket(sendMessage, packet.getMessage());
		sendPacket(sendMessage, table.getNextHop(packet.getDst()));
	}

	// 处理响应包
	void handleResponsePacket(virtualPacket packet) {
		char dstip[20];
		char source[20];
		strcpy(dstip, packet.getDst().ipaddress);
		strcpy(source, packet.getSource().ipaddress);
		if (strcmp(dstip, localaddr) == 0) {
			cout << "packet to IP : " << source << " is received" << endl;
		}
	}

	void sendPacket(char *sendMessage, char *dst) {
		SOCKADDR_IN addr_Server; //服务器的地址等信息
		addr_Server.sin_family = AF_INET;
		addr_Server.sin_port = htons(PORT);
		addr_Server.sin_addr.S_un.S_addr = dst;

		sendto(sock, sendMessage, strlen(sendMessage), 0, (SOCKADDR*)&addr_Server, sizeof(SOCKADDR));
	}

	~controller() {
		closesocket(sock);
		WSACleanup();
		cout << "Server Socketreleased" << endl;
	}
};