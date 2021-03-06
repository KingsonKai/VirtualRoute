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
#include "RouteTableCode/RouteTableDV.cpp"
#include "virtualPacket.cpp"
#define PORT 8080


char ip[SIZE] = "192.168.199.103";
char localname = 'A';


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

	RouteTableDV table;

	// LS
    //RouteTableLS table;

	SOCKET sock;              // socket模块
	sockaddr_in sockAddr;         // 绑定的socket地址
	sockaddr_in sockClient;


	// 最后一次收到邻居发送的心跳包的时间
	vector<pair<Addr, time_t>> heartBeatTimetable;

	// 检测邻居存活


	controller(char *localaddr, char name, int port) {
		strcpy(this->localaddr, localaddr);
		this->port = port;
		this->name = name;
		table = RouteTableDV(name);
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
			int ret = recvfrom(sock, recvBuf, MAXBYTE, 0, (SOCKADDR*)&sockClient, &nSize);
			if (ret > 0) {
			    recvBuf[ret] = '\0';
				handleReceivedPacket(recvBuf);
			}
		}
	}

    void checkNeighbor() {
		time_t currentTime;
		time(&currentTime);
		bool isChange = false;
		for (auto p : heartBeatTimetable) {
			double timeDiff = difftime(currentTime, p.second);
			if (timeDiff > 2.0) {
                cout << "Router is down: " << table.getHostName(p.first.ipaddress) << endl;
				if (table.setdown(table.getHostName(p.first.ipaddress))) {
					isChange = true;
					//sendDownPacket(p.first);
					table.print();
				}
			}
		}
		// DV
		if (isChange) {
			//sendUpdatePacket();
		}
	}

	// 发送普通的packet信息，字符串呀乱七八糟的
	void sendNormalPacket(char *dstip, char *message) {
		char sendMessage[MAXBYTE];
		Addr local(name, localaddr);
		Addr dst('1', dstip);
		virtualPacket normalPacket(0, local, dst, NULL);
		normalPacket.constructNormalPacket(sendMessage, message);
		if (strcmp("0.0.0.0", table.getNextHop(dst)) == 0) {
            cout << dstip << " Can't reach" << endl;
		}
		sendPacket(sendMessage, table.getNextHop(dst));
	}

	void sendUpdatePacket() {
		for (auto addr : table.getNeighbors()) {
			char sendMessage[MAXBYTE];
			Addr local(name, localaddr);
			Addr dst(addr.name, addr.ipaddress);
			virtualPacket updatePacket(1, local, dst, NULL);
			// 向邻居发送更新的距离矢量
			updatePacket.constructRouterInfoPacket(sendMessage, table.get_my_dis_vector(table.getHostName(dst.ipaddress)));
			cout << "Send Update Packet: " << sendMessage << endl;
			sendPacket(sendMessage, table.getNextHop(dst));
		}
	}

	// 向其他路由路由器发送心跳监测包，监测邻居是否被down掉
	void sendHeartBeatPacket() {
		for (auto addr : table.getNeighbors()) {
			char sendMessage[MAXBYTE];
			Addr local(name, localaddr);
			Addr dst(addr.name, addr.ipaddress);
			virtualPacket heartbeatPacket(2, local, dst, NULL);
			heartbeatPacket.constructHeartBeatPacket(sendMessage);
			sendPacket(sendMessage, dst.ipaddress);
		}
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
	/*
	void sendDownPacket(Addr downHost) {
		for (auto addr : table.getNeighbors()) {
			char sendMessage[MAXBYTE];
			Addr dst(addr.name, addr.ipaddress);
			virtualPacket downPacket(4, downHost, dst, NULL);
			downPacket.constructDownPacket(sendMessage);
			// 目的地是邻居，目的IP直接填目的地址
			sendPacket(sendMessage, dst.ipaddress);
		}
	}
	*/

	// 处理接收的包，根据packet类型调用以下四种处理方式
	void handleReceivedPacket(char *recvBuf) {
		/* 将收到的字符串数据转化为数据包格式 */
		virtualPacket packet;
		packet.makePacket(recvBuf);
		if (packet.type == 0) {
			handleNormalPacket(packet);
		}
		else if (packet.type == 1) {
			//handleUpdatePacket(packet);
		}
		else if (packet.type == 2) {
			handleHeartBeatPacket(packet);
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
			sendResponsePacket(packet.getSource().ipaddress, "I have received");
		}
		else {
			forward(packet);
		}
	}

	// 转发普通的包
	void forward(virtualPacket packet) {
	    cout << "From " << table.getHostName(packet.getSource().ipaddress) <<  "Forward To: " << table.getHostName(packet.getDst().ipaddress) << endl;
		if (strcmp("0.0.0.0", table.getNextHop(packet.getDst())) == 0) {
            cout << packet.getDst().ipaddress << " Can't reach" << endl;
		}
		sendPacket(packet.getRecvBuf(), table.getNextHop(packet.getDst()));
	}

	/* 处理路由表信息更新包
	调用DV算法更新路由表
	向邻居转发更新信息
	*/
	void handleUpdatePacket(virtualPacket packet) {
		char source[20];
		char dst[20];
		strcpy(dst, packet.getDst().ipaddress);
		strcpy(source, packet.getSource().ipaddress);
		if (strcmp(dst, localaddr) == 0) {
            cout << "Received Update Packet From " << packet.getSource().ipaddress << " " << packet.message << endl;
			vector<int> disVector = packet.analyzeUpdatePacket();
			if (table.DValgorithm(disVector, table.getHostName(source))) {
				sendUpdatePacket();
			}
		}
	}

	// 处理心跳检测包
	void handleHeartBeatPacket(virtualPacket packet) {
		// 若有收到，回应主机，目前正在工作
		if (strcmp(packet.getDst().ipaddress, localaddr) == 0) {
            cout << packet.getSource().ipaddress << " Heart Beat" << endl;
			time_t receiveTime;
			time(&receiveTime);
			Addr sourceAddr = packet.getSource();
			updateHeartBeatTimetable(sourceAddr, receiveTime);
		}
	}

	// 更新记录收到的每个邻居的心跳包的最后时间
	void updateHeartBeatTimetable(Addr sourceAddr, time_t receiveTime) {
		for (auto p : heartBeatTimetable) {
			if (p.first == sourceAddr) {
				p.second = receiveTime;
			}
		}
	}

	// 处理响应包
	void handleResponsePacket(virtualPacket packet) {
		if (strcmp(packet.getDst().ipaddress, localaddr) == 0) {
			cout << "packet to IP : " << table.getHostName(packet.getSource().ipaddress) << " is received" << endl;
		}
	}

	// LS
	/* 处理接收到的down包
	根据接收到的down包更新自己的路由表
	并将自己的更新信息转发
	*/
	/*
	void handleDownPacket(virtualPacket packet) {
		char source[20];
		char dst[20];
		strcpy(source, packet.getSource().ipaddress);
		strcpy(dst, packet.getDst().ipaddress);

		// LS
		if (strcmp(dst, localaddr) == 0) {
			if (table.setDown(packet.getSource())) {
				forwardDownPacket(packet);
			}
		}
	}
	*/

	// LS
	/* 转发Down包给邻居
	源IP地址不变,改变目的IP地址
	*/
	/*
	void forwardDownPacket(virtualPacket packet) {
		for (auto addr : table.getNeighbors()) {
			packet.changeDstIP(addr.ipaddress);
			sendPacket(packet.getRecvBuf(), addr.ipaddress);
		}
	}
	*/

	void sendPacket(char *sendMessage, char *dst) {
	    if (strcmp(dst, "0.0.0.0") == 0) {
            cout << "Can't reach! Maybe it's down" << endl;
            return;
	    }
	    cout << "Send TO: " << table.getHostName(dst) << " IP : " << dst << " Content: " << sendMessage << endl;
		SOCKADDR_IN addr_Server; //服务器的地址等信息
		addr_Server.sin_family = AF_INET;
		addr_Server.sin_port = htons(PORT);
		addr_Server.sin_addr.S_un.S_addr = inet_addr(dst);

		sendto(sock, sendMessage, strlen(sendMessage), 0, (SOCKADDR*)&addr_Server, sizeof(SOCKADDR));
	}

	~controller() {
		closesocket(sock);
		WSACleanup();
		cout << "Server Socketreleased" << endl;
	}
};

controller c(ip, localname, PORT);

void iniHearbeatTimeTable() {
    time_t currentTime;
    time(&currentTime);
    for (int i = 0; i < 5; i++) {
        c.heartBeatTimetable.push_back(make_pair(c.table.hostAddrs[i], currentTime));
    }
}

void *start(void *args) {
    c.listen();
    c.run();
}

void *send(void *args) {
    while (1) {
        Sleep(5000);
        srand((unsigned)time(NULL));

        int n = rand() % 5;
        while (n == localname - 'A') {
            n = rand() % 5;
        }
        c.sendNormalPacket(c.table.hostAddrs[n].ipaddress, "TEST PACKET");
    }
}

void *heartBeat(void *args) {
    int n = 0;
    while(1) {
        n++;
        Sleep(5000);
        c.sendHeartBeatPacket();
        if (n > 5)
            c.checkNeighbor();
    }
}

int main() {

    iniHearbeatTimeTable();

    pthread_t tids[5];

	pthread_create(&tids[0], NULL, start, NULL);

	// 线程2
    // 每隔5s发一次普通包
	pthread_create(&tids[1], NULL, send, NULL);

	pthread_create(&tids[2], NULL, heartBeat, NULL);

    pthread_exit(NULL);

    return 0;

}
