
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
#include "../RouteTableCode/RouteTableLS.cpp"
#include "../virtualPacket.cpp"
#define PORT 8888

char ip[SIZE] = "127.000.000.001";
char centerName = 'A';

// controller
// ����packet��������Ǹ��Լ��ģ�����������ת��
// ÿ�����뷢��һ��·����Ϣ���Լ��������鿴�Լ����ھ��Ƿ��ڻ���漰down���������
// ����packet������ȷ���յ�����Ϣ
//


class center
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
	RouteTableLS table[5];

	SOCKET sock;                  // socketģ��
	sockaddr_in sockAddr;         // �󶨵�socket��ַ
	sockaddr_in sockClient;


	// ���һ���յ��ھӷ��͵���������ʱ��
	vector<pair<Addr, time_t>> heartBeatTimetable;

	center(char *localaddr, char name, int port) {
		strcpy(this->localaddr, localaddr);
		this->port = port;
		this->name = name;
        for (int i = 0; i < 5; i++) {
            table[i] = RouteTableLS(char(i+'A'));
        }
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
                cout << strlen(recvBuf) << " ";
                cout << recvBuf << endl;
				handleReceivedPacket(recvBuf);
			}
		}
	}

		// ����ھӴ��
	void checkNeighbor() {
		time_t currentTime;
		time(&currentTime);
		bool isChange = false;
		for (auto p : heartBeatTimetable) {
			double timeDiff = difftime(currentTime, p.second);
			if (timeDiff > 2.0) {
                for (int i = 0; i < 5; i++){
                    table[i].setDown(p.first);
                }
			}
		}
	}

	// �������յİ�������packet���͵����������ִ�����ʽ
	void handleReceivedPacket(char *recvBuf) {
		/* ���յ����ַ�������ת��Ϊ���ݰ���ʽ */
		virtualPacket packet;
		packet.makePacket(recvBuf);
		if (packet.type == 2) {
            handleHeartBeatPacket(packet);
		}
        else if (packet.type == 4) {
			handleDownPacket(packet);
		}
		else if (packet.type == 5) {
            handleRequestPacket(packet);
		}
	}

	/* ����·�ɱ���Ϣ���°�
	����DV�㷨����·�ɱ�
	���ھ�ת��������Ϣ
	*/
	/*
	void handleUpdatePacket(virtualPacket packet) {
		char source[20];
		char dst[20];
		strcpy(dst, packet.getDst().ipaddress);
		strcpy(source, packet.getSource().ipaddress);
		char routename = table[0].getHostName(source);
		if (strcmp(dst, localaddr) == 0) {
			vector<int> disVector = packet.analyzeUpdatePacket();
			if (table[routename-'A'].DValgorithm(disVector, table.getHostName(source))) {
				sendUpdatePacket();
			}
		}
	}
	*/

	// ������������
	void handleHeartBeatPacket(virtualPacket packet) {
        time_t receiveTime;
        time(&receiveTime);
        Addr sourceAddr = packet.getSource();
        updateHeartBeatTimetable(sourceAddr, receiveTime);
	}

	// ���¼�¼�յ���ÿ���ھӵ������������ʱ��
	void updateHeartBeatTimetable(Addr sourceAddr, time_t receiveTime) {
		for (auto p : heartBeatTimetable) {
			if (p.first == sourceAddr) {
				p.second = receiveTime;
			}
		}
	}

	// LS
	/* �������յ���down��
	���ݽ��յ���down�������Լ���·�ɱ�
	�����Լ��ĸ�����Ϣת��
	*/
	void handleDownPacket(virtualPacket packet) {
		char source[20];
		char dst[20];
		strcpy(source, packet.getSource().ipaddress);
		strcpy(dst, packet.getDst().ipaddress);

		// LS
		if (strcmp(dst, localaddr) == 0) {
			for (int i = 0; i < 5; i++) {
                table[i].setDown(packet.getSource());
			}
		}
	}

	void handleRequestPacket(virtualPacket packet) {
        char routername = table[0].getHostName(packet.getDst().ipaddress);
        cout << "Routename: " << routername << endl;
        char nextHop[16];
        strcpy(nextHop, table[routername-'A'].getNextHop(packet.getDst()));
        cout << nextHop << endl;
        int len = sizeof(SOCKADDR);
        sendto(sock, nextHop, strlen(nextHop), 0, (SOCKADDR*)&sockClient, len);
	}

	~center() {
		closesocket(sock);
		WSACleanup();
		cout << "Server Socketreleased" << endl;
	}
};

center c(ip, centerName, PORT);


void *start(void *args) {
    c.listen();
    c.run();
}

void *check(void *args) {
    c.checkNeighbor();
}

int main() {

    pthread_t tids[5];

	pthread_create(&tids[0], NULL, start, NULL);
	pthread_create(&tids[1], NULL, check, NULL);

    pthread_exit(NULL);

    return 0;

}