#include<vector>
#include<iostream>
#include<map>
#include<queue>
#include "DataStructure.cpp"
using namespace std;
class RouteTableDV
{
public:
	vector<routeTableEntry> routetable;//目标，下一跳，cost
	vector<vector<int>> myTable;
	map<char, vector<pathInfo>> networkGraph;//只有自己到自己邻居的边
	vector<Addr> hostAddrs;
	char myHostName;//本机主机名 
	RouteTableDV(char name) {
		myTable = vector<vector<int>>(5, vector<int>(5, 9999));
		myHostName = name;
		//获取所有hostAddrs,0对应A的信息
		hostAddrs.push_back(Addr('A', "172.18.157.159"));
		hostAddrs.push_back(Addr('B', "172.18.156.76"));
		hostAddrs.push_back(Addr('C', "172.18.159.66"));
		hostAddrs.push_back(Addr('D', "172.18.159.150"));
		hostAddrs.push_back(Addr('E', "172.18.158.165"));
		//获取到邻居的边,不同主机不一样，去LS那边复制一下就可以了
		/*
		networkGraph['C'] = vector<pathInfo>{
			pathInfo(hostAddrs[2], hostAddrs[0], 3),
			pathInfo(hostAddrs[2], hostAddrs['D' - 'A'], 2)
		};
		networkGraph['D'] = vector<pathInfo>{
			pathInfo(hostAddrs[3], hostAddrs[0], 7),
			pathInfo(hostAddrs[3], hostAddrs['C' - 'A'], 2),
			pathInfo(hostAddrs[3], hostAddrs['B' - 'A'], 1),
		};
		networkGraph['E'] = vector<pathInfo>{
			pathInfo(hostAddrs[4], hostAddrs[0], 5),
			pathInfo(hostAddrs[4], hostAddrs['B' - 'A'], 3)
		};
		*/
		//初始自己的路由表
		if (myHostName == 'A') {
			networkGraph['A'] = vector<pathInfo>{
				pathInfo(hostAddrs[0], hostAddrs['C' - 'A'], 3),
				pathInfo(hostAddrs[0], hostAddrs['D' - 'A'], 7),
				pathInfo(hostAddrs[0], hostAddrs['E' - 'A'], 5),
			};
		}
		else if (myHostName == 'B') {
			networkGraph['B'] = vector<pathInfo>{
				pathInfo(hostAddrs[1], hostAddrs['D' - 'A'], 1),
				pathInfo(hostAddrs[1], hostAddrs['E' - 'A'], 2)
			};
		}
		else if (myHostName == 'C') {
			networkGraph['C'] = vector<pathInfo>{
				pathInfo(hostAddrs[2], hostAddrs['A' - 'A'], 3),
				pathInfo(hostAddrs[2], hostAddrs['D' - 'A'], 2)
			};
		}
		else if (myHostName == 'D') {
			networkGraph['D'] = vector<pathInfo>{
				pathInfo(hostAddrs[3], hostAddrs['A' - 'A'], 7),
				pathInfo(hostAddrs[3], hostAddrs['C' - 'A'], 2),
				pathInfo(hostAddrs[3], hostAddrs['B' - 'A'], 1)
			};
		}
		else {
			networkGraph['E'] = vector<pathInfo>{
				pathInfo(hostAddrs[4], hostAddrs['A' - 'A'], 5),
				pathInfo(hostAddrs[4], hostAddrs['B' - 'A'], 3)
			};
		}

		for	(int i = 0; i < networkGraph[myHostName].size(); i++) {
			myTable[myHostName - 'A'][networkGraph[myHostName][i].addr2.name - 'A'] = networkGraph[myHostName][i].cost;//邻居的加进路由，下一跳先不考虑
			myTable[networkGraph[myHostName][i].addr2.name - 'A'][myHostName - 'A'] = networkGraph[myHostName][i].cost;
			routetable.push_back(routeTableEntry(networkGraph[myHostName][i].addr2, networkGraph[myHostName][i].addr2.ipaddress, networkGraph[myHostName][i].cost));
		}
		myTable[myHostName - 'A'][myHostName - 'A'] = 0;
	}
	~RouteTableDV() {}
	void DValgorithm(vector<vector<int>> anTable, char anHostName) {// 更新路由表，传入的是某个邻居发来的路由表以及它的主机名
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 5; j++) {
				if (anTable[i][j] < myTable[i][j]) myTable[i][j] = anTable[i][j];
			}
		}

		for (int i = 0; i < 5; i++) {//Bellman-Ford算法
			for (int j = 0; j < 5; j++) {
				for (int z = 0; z < 5; z++) {
					if (myTable[i][z] + myTable[z][j] < myTable[i][j]) {
						myTable[i][j] = myTable[i][z] + myTable[z][j];
						if (i + 'A' == myHostName) {
							if (routetable.empty()) {
								Addr destAddr = Addr(hostAddrs[j]);
								char nextRoute[16];
								strcpy_s(nextRoute, hostAddrs[z].ipaddress);
								routetable.push_back(routeTableEntry(destAddr, nextRoute, myTable[i][j]));
							}
							else {
								bool is = false;
								for (int k = 0; k < routetable.size(); k++) {
									if (routetable[k].addr.name == hostAddrs[j].name) {
										cout << "D in" << endl;
										strcpy_s(routetable[k].nexthop, hostAddrs[z].ipaddress);
										routetable[k].cost = myTable[i][j];
										is = true;
										break;
									}
								}
								if (!is) {
									Addr destAddr = Addr(hostAddrs[j]);
									char nextRoute[16];
									strcpy_s(nextRoute, hostAddrs[z].ipaddress);
									routetable.push_back(routeTableEntry(destAddr, nextRoute, myTable[i][j]));
								}
							}
						}
					}
				}
			}
		}



		/*int neighbor_num = 0;
		for (int neighbor_num = 0; neighbor_num < routetable.size(); neighbor_num++) {
		if (routetable[neighbor_num].addr.name == anHostName) {
		break;
		}
		}
		for (int i = 0; i < anRouteTable.size(); i++) {//算法核心
		int j = 0;
		for (j = 0; j < routetable.size(); j++) {
		if (routetable[j].addr.name == anRouteTable[i].addr.name) {
		if (routetable[j].cost > anRouteTable[i].cost + routetable[neighbor_num].cost) routetable[j] = routeTableEntry(routetable[j].addr.name, routetable[neighbor_num].nexthop, anRouteTable[i].cost + routetable[neighbor_num].cost);
		break;
 		}
		}
		if (j == routetable.size()) routetable.push_back(routeTableEntry(anRouteTable[i].addr.name, routetable[neighbor_num].nexthop ,anRouteTable[i].cost + routetable[neighbor_num].cost));
		}*/
	}
	char* getNextHop(Addr dst) {   // 获得下一跳路由器
		for (int i = 0; i < routetable.size(); i++) {
			if (routetable[i].addr.name == dst.name) return routetable[i].nexthop;
		}
		return ("0.0.0.0");//默认返回
	}
	void print() {
		for (auto v : routetable) {
			cout << "destName: " << v.addr.name << endl;
			cout << "NExtIP: " << v.nexthop << endl;
			cout << "Cost: " << v.cost << endl;
		}
	}
};
/*
int main() {
	cout << "hello world" << endl;

	RouteTableDV routeA = RouteTableDV('A');
	RouteTableDV routeC = RouteTableDV('C');
	RouteTableDV routeD = RouteTableDV('D');
	RouteTableDV routeE = RouteTableDV('E');
	routeA.DValgorithm(routeC.myTable, 'C');

	routeA.DValgorithm(routeD.myTable, 'D');
	routeA.DValgorithm(routeE.myTable, 'E');
	routeA.print();


	return 0;
}
*/