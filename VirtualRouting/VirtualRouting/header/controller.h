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
	char localaddr[SIZE];          // ������ַ
    char name;
	int port;                      // �˿ں�
	std::vector<route> routelist;  // �ھ�·�ɱ�
	RouteTableLS table;

	SOCKET sock;                  // socketģ��
	sockaddr_in sockAddr;         // �󶨵�socket��ַ

	controller(char *localaddr, char name, int port, vector<route> routelist);


	void listen();   // ��socket
	void run();      // start����
	void sendHeartBeatPacket();   // ����������·��������������ⱨ�ģ�����ھ��Ƿ�down��
	void sendUpdatePacket();      // ����·�ɱ���Ϣ
	void sendNormalPacket(char *dstip, char *message);
    void sendResponsePacket(char *dstip, char *RESPONSE);  // ���ͻ�����Ϣ�������Ƿ��յ����������
	void handleReceivedPacket(char *recvBuf);          // ������յı��� ����packet���͵����������ִ�����
	void handleHeartBeatPacket(virtualPacket packet);  // ����������ⱨ��
	void handleUpdatePacket(virtualPacket packet);     // ����·�ɱ���Ϣ���±���
	void handleNormalPacket(virtualPacket packet);     // ������ͨ����
	void forward(virtualPacket packet);                // ת������
	void handleResponsePacket(virtualPacket packet);   // ������Ӧ����

	SOCKADDR_IN sendPacket(char *sendMessage, char *dst);

	~controller();
};
