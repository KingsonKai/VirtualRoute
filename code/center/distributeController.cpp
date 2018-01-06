#include <Winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <ctime>
#define HAVE_STRUCT_TIMESPEC
#include <windows.h>
#include <pthread.h>
// vs 忽略strcpy安全性问题
#pragma warning(disable:4996)
using namespace std;
#include "../RouteTableCode/RouteTableLS.cpp"
#include "../virtualPacket.cpp"
#define PORT 8080
#define centerPORT 8888

char ip[SIZE] = "127.000.000.001";
char centerIP[SIZE] = "127.000.000.001";
char localname = 'A';
pthread_mutex_t work_mutex;   // 声明互斥变量

// controller
// 接收packet，如果刚是给自己的，解析，否则转发
// 每隔两秒发送一次路由信息，以及侦测包，查看自己的邻居是否还在活动（涉及down掉的情况）
// 发送packet，比如确认收到等信息
//


class controller
{
public:
	char localaddr[SIZE];         // 本机地址
	char name;
	int port;                     // 端口号

	SOCKET sock;              // socket模块
	sockaddr_in sockAddr;         // 绑定的socket地址


	// 最后一次收到邻居发送的心跳包的时间
	vector<pair<Addr, time_t>> heartBeatTimetable;

	controller(char *localaddr, char name, int port) {
		strcpy(this->localaddr, localaddr);
		this->port = port;
		this->name = name;
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
            SOCKADDR_IN sockClient;
			int nSize = sizeof(sockAddr);

			char recvBuf[MAXBYTE];
			int ret = recvfrom(sock, recvBuf, MAXBYTE, 0, (SOCKADDR*)&sockClient, &nSize);
			if (ret > 0) {
			    recvBuf[ret] = '\0';
			    cout << recvBuf << endl;
                cout << strlen(recvBuf) << endl;
				handleReceivedPacket(recvBuf);
			}
			// getPack or forward packet
		}
	}

	// 发送普通的packet信息，字符串呀乱七八糟的
	void sendNormalPacket(char *dstip, char *message) {
		char sendMessage[MAXBYTE];
		Addr local(name, localaddr);
		Addr dst('1', dstip);
		virtualPacket normalPacket(0, local, dst, NULL);
		normalPacket.constructNormalPacket(sendMessage, message);
		sendPacket(sendMessage, getNextHop(dst));
	}

	// 向其他路由路由器发送心跳监测包，监测邻居是否被down掉
	void sendHeartBeatPacket() {
        char sendMessage[MAXBYTE];
        Addr local(name, localaddr);
        Addr dst('O', centerIP);
        virtualPacket heartbeatPacket(2, local, dst, NULL);
        heartbeatPacket.constructHeartBeatPacket(sendMessage);
        sendPacketToCenter(sendMessage);
	}

	// 发送回响信息，表明是否收到了这个包
	void sendResponsePacket(char *dstip, char *RESPONSE) {
		char sendMessage[MAXBYTE];
		Addr local(name, localaddr);
		Addr dst('1', dstip);
		virtualPacket responsePacket(3, local, dst, NULL);
		char message[MAXBYTE];
		strcpy(message, RESPONSE);
		responsePacket.constructResponsePacket(sendMessage, message);
		sendPacket(sendMessage, dst.ipaddress);
	}

	// LS
	// down包只发送给邻居
	void sendDownPacket(Addr downHost) {
        char sendMessage[MAXBYTE];
        Addr dst('0', centerIP);
        virtualPacket downPacket(4, downHost, dst, NULL);
        downPacket.constructDownPacket(sendMessage);
        sendPacketToCenter(sendMessage);
	}

	// 处理接收的包，根据packet类型调用以下四种处理方式
	void handleReceivedPacket(char *recvBuf) {
		/* 将收到的字符串数据转化为数据包格式 */
		virtualPacket packet;
		packet.makePacket(recvBuf);
		if (packet.type == 0) {
			handleNormalPacket(packet);
		}
		else if (packet.type == 3) {
			handleResponsePacket(packet);
		}
		/*
		else if (packet.type == 4) {
			handleDownPacket(packet);
		}
		*/
	}

	/* 处理普通的包
	如果目的地址是自己则将包的内容输出
	否则根据目的地址转发
	*/
	void handleNormalPacket(virtualPacket packet) {
		if (strcmp(packet.getDst().ipaddress, localaddr) == 0) {
			packet.print();
			sendResponsePacket(packet.getSource().ipaddress, "I have received");
		}
		else {
			forward(packet);
		}
	}

	// 转发普通的包
	void forward(virtualPacket packet) {
	    cout << "From " << packet.getSource().ipaddress <<  "Forward To: " << packet.getDst().ipaddress << endl;
		sendPacket(packet.getRecvBuf(), getNextHop(packet.getDst()));
	}

	// 处理响应包
	void handleResponsePacket(virtualPacket packet) {
		if (strcmp(packet.getDst().ipaddress, localaddr) == 0) {
			cout << "packet to IP : " << packet.getSource().ipaddress << " is received" << endl;
		}
	}

	SOCKADDR_IN sendPacket(char *sendMessage, char *dst) {
	    if (strcmp(dst, "0.0.0.0") == 0) {
            cout << "Can't reach! Maybe it's down" << endl;
	    }
		SOCKADDR_IN addr_Server; //服务器的地址等信息
		addr_Server.sin_family = AF_INET;
		addr_Server.sin_port = htons(PORT);
		addr_Server.sin_addr.S_un.S_addr = inet_addr(dst);

		sendto(sock, sendMessage, strlen(sendMessage), 0, (SOCKADDR*)&addr_Server, sizeof(SOCKADDR));
		return addr_Server;
	}

	char *getNextHop(Addr a) {
		Addr local(name, localaddr);
		virtualPacket request(5, local, a, "requestnexthop");
        char sendMessage[MAXBYTE];
        request.constructRequestPacket(sendMessage);
        auto addr_Server = sendPacketToCenter(sendMessage);

        int len = sizeof(SOCKADDR);
        char recvBuf[MAXBYTE];
        int ret = recvfrom(sock, recvBuf, MAXBYTE, 0,(SOCKADDR*)&addr_Server, &len);
        recvBuf[ret] = '\0';
        cout << "Request Next hop is: " << recvBuf << endl;
        return recvBuf;
	}

	SOCKADDR_IN sendPacketToCenter(char *sendMessage) {
        SOCKADDR_IN addr_Server;
        int nSize = sizeof(addr_Server);
        addr_Server.sin_family = AF_INET;
		addr_Server.sin_port = htons(centerPORT);
		addr_Server.sin_addr.S_un.S_addr = inet_addr(centerIP);
        sendto(sock, sendMessage, strlen(sendMessage), 0, (SOCKADDR*)&addr_Server, nSize);

        return addr_Server;
	}

	~controller() {
		closesocket(sock);
		WSACleanup();
		cout << "Server Socketreleased" << endl;
	}
};

controller c(ip, localname, PORT);
vector<Addr> hostAddrs;

void *start(void *args) {
    c.listen();
    c.run();
}

void *heartBeat(void *args) {
    int n = 0;
    while(1) {
        n++;
        Sleep(2000);
        if (n > 5)
            c.sendHeartBeatPacket();
    }
}

void ini() {
    char ipA[SIZE] = "172.018.157.159";
    char ipB[SIZE] = "172.018.156.076";
    char ipC[SIZE] = "172.018.159.066";
    char ipD[SIZE] = "172.018.159.150";
    char ipE[SIZE] = "172.018.158.165";
    hostAddrs.push_back(Addr('A', ipA));
    hostAddrs.push_back(Addr('B', ipB));//赋值后是多少位
    hostAddrs.push_back(Addr('C', ipC));
    hostAddrs.push_back(Addr('D', ipD));
    hostAddrs.push_back(Addr('E', ipE));
}

int main() {
    ini();
    pthread_t tids[5];

	pthread_create(&tids[0], NULL, start, NULL);


    // DV算法的发送心跳包
	// pthread_create(&tids[3], NULL, heartBeat, NULL);

    pthread_exit(NULL);

    return 0;

}
