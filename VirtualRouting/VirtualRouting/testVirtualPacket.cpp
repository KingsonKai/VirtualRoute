#include <iostream>
#include <cstring>
#include <vector>
#include <climits>

using namespace std;
#define MAX_BYTE 10000
#define SIZE 16

// 基本的地址信息
class Addr
{
public:
    char name;                 // 如ABCD
    char ipaddress[SIZE];      // IP地址
    Addr(char name, char *ip) {
        strcpy(ipaddress, ip);
        this->name = name;
    }
    
    Addr() {
        name = '1';
    }
};

// 路由表信息单位
class routeTableEntry
{
public:
    Addr addr;
    char nexthop[SIZE];         // 下一跳路由器
    int cost;                   // 路径开销

    routeTableEntry(Addr addr, char nexthop[16], int cost) {
        this->addr = addr;
        strcpy(this->nexthop, nexthop);
        this->cost = cost;
    }
};

class virtualPacket
{
public:
    int type;
    Addr source;
    Addr dst;
    char message[MAX_BYTE];

    virtualPacket(int type, Addr source, Addr dst, char *message) {
        this->type = type;
        this->source = source;
        this->dst = dst;
        strcpy(this->message, message);
    }
    
    virtualPacket() {
        type = -1;
    }

    // 所有要发送的信息，构建好一条字符串，然后复制到sendMessage中去
    // sendMessage是指针，复制后内容会改变，在路由器模块中就能接收到消息

    // 根据要发送的message，构建好字符流，调用strncpy复制到sendMessage中
    void constructNormalPacket(char *sendMessage, char *message) {
        sendMessage[0] = '0';
        writeAddress(sendMessage+1);
        cout << "WriteAddress: " << sendMessage << endl;
        strncpy(sendMessage+33, message, strlen(message));
        cout << "WrtieMessage: " << sendMessage << endl;
        sendMessage[strlen(sendMessage)] = '\0';
    }

    // 打包路由表信息
    void constructRouterInfoPacket(char *sendMessage, std::vector<routeTableEntry> routeTable) {
        sendMessage[0] = '1';
        writeAddress(sendMessage+1);
        char routeMessage[MAX_BYTE];
        for (auto e : routeTable) {
            strncpy(routeMessage+strlen(routeMessage), e.addr.ipaddress, strlen(e.addr.ipaddress));
            if (e.cost >= 10) {
                sprintf(routeMessage+strlen(routeMessage), "%d", e.cost);
            }
            else {
                routeMessage[strlen(routeMessage)] = '0';
                sprintf(routeMessage+strlen(routeMessage), "%d", e.cost);
            }
            routeMessage[strlen(routeMessage)] = '*';
        }
        strncpy(sendMessage+33, routeMessage, strlen(routeMessage));
        sendMessage[strlen(sendMessage)] = '\0';
    }

    // 打包心跳检测信息
    void constructHeartBeatPecket(char *sendMessage) {
        sendMessage[0] = '2';
        writeAddress(sendMessage+1);
        char message[] = "I am alive!";
        strncpy(sendMessage+33, "I am alive!", strlen(message));
    }

    void writeAddress(char *sendMessage) {
        strncpy(sendMessage, source.ipaddress, strlen(source.ipaddress));
        sendMessage[15] = '*';
        strncpy(sendMessage+16, dst.ipaddress, strlen(dst.ipaddress));
        sendMessage[31] = '*';
    }

    // 根据接收到的信息，实例化为一个virtual packet
    void makePacket(char *receivedMessage) {
        type = receivedMessage[0] - '0';
        strncpy(source.ipaddress, receivedMessage+1, 15);
        strncpy(dst.ipaddress, receivedMessage+17, 15);
        source.ipaddress[16] = '\0';
        dst.ipaddress[16] = '\0';
        strncpy(message, receivedMessage+33, strlen(receivedMessage)-32);
    }

    // 输出这个包的内容
    void print() {
        cout << "--------Packet content--------" << endl;
        cout << "--type: " << type << endl;
        cout << "--source ip address: " << source.ipaddress << endl;
        cout << "--destination ip address: " << dst.ipaddress << endl;
        cout << "--content: " << message << endl;
    }

};

int main() {
    virtualPacket test;
    char received[] = "0192.168.111.111*192.168.111.112*I am alive";
    char sendMessage[MAX_BYTE];
    test.makePacket(received);
    char message[] = "HereHere";
    test.constructNormalPacket(sendMessage, message);
    cout << "Normal Packet: " << sendMessage << endl;

    memset(sendMessage, 0, sizeof(sendMessage));
    test.constructHeartBeatPecket(sendMessage);
    cout << "HeartBeat Packet: " << sendMessage << endl;

    memset(sendMessage, 0, sizeof(sendMessage));
    Addr A('A', "192.168.111.111");
    Addr B('B', "192.168.111.112");

    routeTableEntry e1(A, "192.168.111.113", 10);
    routeTableEntry e2(B, "192.168.111.114", 5);

    std::vector<routeTableEntry> r;
    r.push_back(e1);
    r.push_back(e2);
    test.constructRouterInfoPacket(sendMessage, r);

    cout << "Route Information packet: " << sendMessage << endl;
    return 0;
}