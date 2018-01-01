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
	char localaddr[SIZE];         // ������ַ
    char name;
	int port;                     // �˿ں�
	std::vector<route> routelist;  // �ھ�·�ɱ�
	RouteTableLS table;

	SOCKET sock;              // socketģ��
	sockaddr_in sockAddr;         // �󶨵�socket��ַ

	controller(char *localaddr, char name, int port, vector<route> routelist);

	// ��socket
	void listen();

	// start����
	void run();

	// ����������·����������������������ھ��Ƿ�down��
	void sendHeartBeatPacket();
	// ����·�ɱ���Ϣ
	void sendUpdatePacket();

	// ������ͨ��packet��Ϣ���ַ���ѽ���߰����
	void sendNormalPacket(char *dstip, char *message);

	// ���ͻ�����Ϣ�������Ƿ��յ��������
	void sendResponsePacket(char *dstip, char *RESPONSE);

	// ������յİ�������packet���͵����������ִ���ʽ
	void handleReceivedPacket(char *recvBuf);

	// ������������
	void handleHeartBeatPacket(virtualPacket packet);

	// ����·�ɱ���Ϣ���°�
	void handleUpdatePacket(virtualPacket packet);
	// ������ͨ�İ�
	void handleNormalPacket(virtualPacket packet);

	// ת����ͨ�İ�
	void forward(virtualPacket packet);

	// ������Ӧ��
	void handleResponsePacket(virtualPacket packet);

	SOCKADDR_IN sendPacket(char *sendMessage, char *dst);

	~controller();
};
