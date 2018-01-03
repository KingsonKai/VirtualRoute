#include <iostream>
#include <cstring>
#include <vector>
#include <climits>
using namespace std;
#define MAXBYTE 10000
#include "DataStructure.cpp"
#include "virtualPacket.cpp"
#define SIZE 16


int main() {
    virtualPacket test;
    char received[] = "0192.168.111.111*192.168.111.112*I am alive";
    vector<int> v = {1, 2, 3, 4, 5};
    char sendMessage[MAXBYTE];
    test.makePacket(received);
    test.print();
    char message[] = "HereHere";
    test.constructNormalPacket(sendMessage, message);
    cout << "Normal Packet: " << "len: " << strlen(sendMessage) << ' ' << sendMessage << endl;
    memset(sendMessage, 0, sizeof(sendMessage));
    test.constructDownPacket(sendMessage);
    cout << "Down Packet: " << "len: " << strlen(sendMessage) << ' ' << sendMessage << endl;
    memset(sendMessage, 0, sizeof(sendMessage));
    test.constructHeartBeatPacket(sendMessage);
    cout << "HeartBeat Packet: " << "len: " << strlen(sendMessage) << ' ' << sendMessage << endl;
    memset(sendMessage, 0, sizeof(sendMessage));
    test.constructResponsePacket(sendMessage, "Yeah");
    cout << "Response Packet: " << "len: " << strlen(sendMessage) << ' ' << sendMessage << endl;
    memset(sendMessage, 0, sizeof(sendMessage));
    test.constructRouterInfoPacket(sendMessage, v);
    cout << "RouterInfo Packet: " << "len: " << strlen(sendMessage) << ' ' << sendMessage << endl;
    memset(sendMessage, 0, sizeof(sendMessage));

    return 0;
}
