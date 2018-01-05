#include <iostream>
#include <vector>
#include <winsock2.h>
using namespace std;
#include "../DataStructure.cpp"
#include "../virtualPacket.cpp"
#define PORT 8080

vector<Addr> hostAddrs;
int localNum = 0;
char localname = 'A';
char ip[16] = "192.168.191.001";

void sendPacket(char *sendMessage, char *dst) {
    if (strcmp(dst, "0.0.0.0") == 0) {
        cout << "Can't reach! Maybe it's down" << endl;
    }
    SOCKET sock;
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    SOCKADDR_IN addr_Server; //服务器的地址等信息
    addr_Server.sin_family = AF_INET;
    addr_Server.sin_port = htons(PORT);
    addr_Server.sin_addr.S_un.S_addr = inet_addr(dst);

    sendto(sock, sendMessage, strlen(sendMessage), 0, (SOCKADDR*)&addr_Server, sizeof(SOCKADDR));
}

void ini() {
    char ipA[SIZE] = "192.168.191.001";
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

    for (int i = 0; i < 5; i++) {
        if (i != localNum) {
            Addr local(localname, ip);
            Addr dst('-', hostAddrs[i].ipaddress);
            virtualPacket normalPacket(0, local, dst, NULL);
            char sendMessage[MAXBYTE];
            normalPacket.constructNormalPacket(sendMessage, "TEST PACKET");
            cout << sendMessage << endl;
            sendPacket(sendMessage, ip);
        }
    }
}
