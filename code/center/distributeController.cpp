#include <Winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <ctime>
#define HAVE_STRUCT_TIMESPEC
#include <windows.h>
#include <pthread.h>
// vs ����strcpy��ȫ������
#pragma warning(disable:4996)
using namespace std;
#include "RouteTableCode/RouteTableLS.cpp"
#include "virtualPacket.cpp"
#define PORT 8080

char ip[SIZE] = "127.000.000.001";
char centerIP[SIZE] = "127.000.000.001";
char localname = 'A';

// controller
// ����packet��������Ǹ��Լ��ģ�����������ת��
// ÿ�����뷢��һ��·����Ϣ���Լ��������鿴�Լ����ھ��Ƿ��ڻ���漰down���������
// ����packet������ȷ���յ�����Ϣ
//


class controller
{
public:
	char localaddr[SIZE];         // ������ַ
	char name;
	int port;                     // �˿ں�

    /*
	// DV
	RouteTableDV table;			   //RouteTableLS table;
    */

	// LS
	RouteTableLS table;

	SOCKET sock;              // socketģ��
	sockaddr_in sockAddr;         // �󶨵�socket��ַ
	sockaddr_in sockClient;


	// ���һ���յ��ھӷ��͵���������ʱ��
	vector<pair<Addr, time_t>> heartBeatTimetable;

	controller(char *localaddr, char name, int port) {
		strcpy(this->localaddr, localaddr);
		this->port = port;
		this->name = name;
		table = RouteTableLS(name);
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}

	// ��socket
	void listen() {
		memset(&sockAddr, 0, sizeof(sockAddr));
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_addr.s_addr = inet_addr(localaddr);
		sockAddr.sin_port = htons(port);
		bind(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
		cout << "Listen at PORT " << port << endl;
	}

	// start����
	void run() {
		while (1) {
			int nSize = sizeof(sockAddr);

			char recvBuf[MAXBYTE];
			int ret = recvfrom(sock, recvBuf, MAXBYTE, 0, (SOCKADDR*)&sockClient, &nSize);
			if (ret > 0) {
			    recvBuf[ret] = '\0';
                cout << strlen(recvBuf) << endl;
				handleReceivedPacket(recvBuf);
			}
			// getPack or forward packet
		}
	}

	// ������ͨ��packet��Ϣ���ַ���ѽ���߰����
	void sendNormalPacket(char *dstip, char *message) {
		char sendMessage[MAXBYTE];
		Addr local(name, localaddr);
		Addr dst('1', dstip);
		virtualPacket normalPacket(0, local, dst, NULL);
		normalPacket.constructNormalPacket(sendMessage, message);
		sendPacket(sendMessage, table.getNextHop(dst));
	}

	// ������·��·����������������������ھ��Ƿ�down��
	void sendHeartBeatPacket() {
		for (auto addr : table.getNeighbors()) {
			char sendMessage[MAXBYTE];
			Addr local(name, localaddr);
			Addr dst(addr.name, addr.ipaddress);
			virtualPacket heartbeatPacket(2, local, dst, NULL);
			heartbeatPacket.constructHeartBeatPacket(sendMessage);
			sendPacket(sendMessage, table.getNextHop(dst));
		}
	}

	// ���ͻ�����Ϣ�������Ƿ��յ��������
	void sendResponsePacket(char *dstip, char *RESPONSE) {
		char sendMessage[MAXBYTE];
		Addr local(name, localaddr);
		Addr dst('1', dstip);
		virtualPacket responsePacket(3, local, dst, NULL);
		char message[MAXBYTE];
		strcpy(message, RESPONSE);
		responsePacket.constructResponsePacket(sendMessage, message);
		cout << sendMessage << endl;
		sendPacket(sendMessage, dst.ipaddress);
	}

	// LS
	// down��ֻ���͸��ھ�
	void sendDownPacket(Addr downHost) {
		for (auto addr : table.getNeighbors()) {
			char sendMessage[MAXBYTE];
			Addr dst(addr.name, addr.ipaddress);
			virtualPacket downPacket(4, downHost, dst, NULL);
			downPacket.constructDownPacket(sendMessage);
			// Ŀ�ĵ����ھӣ�Ŀ��IPֱ����Ŀ�ĵ�ַ
			sendPacket(sendMessage, dst.ipaddress);
		}
	}

	// ������յİ�������packet���͵����������ִ���ʽ
	void handleReceivedPacket(char *recvBuf) {
		/* ���յ����ַ�������ת��Ϊ���ݰ���ʽ */
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

	/* ������ͨ�İ�
	���Ŀ�ĵ�ַ���Լ��򽫰����������
	�������Ŀ�ĵ�ַת��
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

	// ת����ͨ�İ�
	void forward(virtualPacket packet) {
	    cout << "From " << packet.getSource().ipaddress <<  "Forward To: " << packet.getDst().ipaddress << endl;
		sendPacket(packet.getRecvBuf(), table.getNextHop(packet.getDst()));
	}

	// ������Ӧ��
	void handleResponsePacket(virtualPacket packet) {
		if (strcmp(packet.getDst().ipaddress, localaddr) == 0) {
			cout << "packet to IP : " << packet.getSource().ipaddress << " is received" << endl;
		}
	}

	SOCKADDR_IN sendPacket(char *sendMessage, char *dst) {
		SOCKADDR_IN addr_Server; //�������ĵ�ַ����Ϣ
		addr_Server.sin_family = AF_INET;
		addr_Server.sin_port = htons(PORT);
		addr_Server.sin_addr.S_un.S_addr = inet_addr(dst);

		sendto(sock, sendMessage, strlen(sendMessage), 0, (SOCKADDR*)&addr_Server, sizeof(SOCKADDR));
		return addr_Server;
	}

	~controller() {
		closesocket(sock);
		WSACleanup();
		cout << "Server Socketreleased" << endl;
	}
};

controller c(ip, localname, PORT);

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

void *down() {
    Sleep(10000);
    c.sendDownPacket(Addr(localname, ip));
}

void *heartBeat() {
    Sleep(2000);
    c.sendHeartBeatPacket();
}

int main() {

    pthread_t tids[5];

	pthread_create(&tids[0], NULL, start, NULL);

	// �߳�2
    // ÿ��5s��һ����ͨ��
	pthread_create(&tids[1], NULL, send, NULL);

    /*
    // �߳�3, ����down��
	pthread_create(&tids[2], NULL, down, NULL);


    // DV�㷨�ķ���������
	pthread_create(&tids[3], NULL, heartBeat, NULL);
	*/
    pthread_exit(NULL);

    return 0;

}
