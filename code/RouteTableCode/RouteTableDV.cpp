#include<vector>
#include<iostream>
#include<map>
#include<string>
#include<queue>
#include<iterator>
// vs 忽略strcpy安全性问题
#pragma warning(disable:4996)
#include "../DataStructure.cpp"
using namespace std;

class RouteTableDV {
public:
	vector<routeTableEntry> routetable;//目标，下一跳，cost
	vector<vector<int>> myTable; // 路由选择表
	vector<int> Dis_vector; //自己的距离向量
	map<char, vector<pathInfo>> networkGraph;//只有自己到自己邻居的边
	vector<Addr> hostAddrs; // IP和地址的映射
	char myHostName;//本机主机名
	RouteTableDV(char name = 'A') {
		myTable = vector<vector<int>>(5, vector<int>(5, 9999));
		Dis_vector = vector<int>(5, 9999);
		myHostName = name;

		//获取所有hostAddrs,0对应A的信息
		char ipA[SIZE] = "192.168.199.103";
		char ipB[SIZE] = "192.168.199.122";
		char ipC[SIZE] = "192.168.199.160";
		char ipD[SIZE] = "192.168.199.231";
		char ipE[SIZE] = "192.168.199.198";
		hostAddrs.push_back(Addr('A', ipA));
		hostAddrs.push_back(Addr('B', ipB));//赋值后是多少位
		hostAddrs.push_back(Addr('C', ipC));
		hostAddrs.push_back(Addr('D', ipD));
		hostAddrs.push_back(Addr('E', ipE));

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
		//对自己的路由选择表进行初始化，将自己到邻居的边加入路由选择表中，同时更新路由转发表
		for (int i = 0; i < networkGraph[myHostName].size(); i++) {
			myTable[myHostName - 'A'][networkGraph[myHostName][i].addr2.name - 'A'] = networkGraph[myHostName][i].cost;//邻居的加进路由，下一跳先不考虑
			myTable[networkGraph[myHostName][i].addr2.name - 'A'][myHostName - 'A'] = networkGraph[myHostName][i].cost;
			routetable.push_back(routeTableEntry(networkGraph[myHostName][i].addr2, networkGraph[myHostName][i].addr2.ipaddress, networkGraph[myHostName][i].cost));
			Dis_vector[networkGraph[myHostName][i].addr2.name - 'A'] = networkGraph[myHostName][i].cost;
		}
		myTable[myHostName - 'A'][myHostName - 'A'] = 0;
		Dis_vector[myHostName - 'A'] = 0;
	}
	~RouteTableDV() {}

	bool DValgorithm(vector<int> Nei_dis, char anHostName) {// 更新路由表，传入的是某个邻居发来的路由表以及它的主机名
		vector<vector<int> > origin_table;
		copy(origin_table, myTable);
		//将邻居新传来的距离向量更新自自己的路由选择表
		for (int i = 0; i < hostAddrs.size(); i++) {
			myTable[anHostName - 'A'][i] = Nei_dis[i];
		}
		for (int i = 0; i < hostAddrs.size(); i++) {//Bellman-Ford方程
			for (int j = 0; j < hostAddrs.size(); j++) {
				for (int z = 0; z < hostAddrs.size(); z++) {
					if (myTable[i][z] + myTable[z][j] < myTable[i][j]) {
						myTable[i][j] = myTable[i][z] + myTable[z][j];
						//若本机的距离向量改变，则更新路由转发表
						if (i + 'A' == myHostName && myTable[i][j] < 9999) {
							//若路由转发表为空，直接加入
							if (routetable.empty()) {
								Addr destAddr = Addr(hostAddrs[j]);
								char nextRoute[16];
								strcpy(nextRoute, hostAddrs[z].ipaddress);
								routetable.push_back(routeTableEntry(destAddr, nextRoute, myTable[i][j]));
							}
							else {
								bool is = false;
								//若路由转发表中已经存在目的地，则直接修改路由转发表中的信息
								for (int k = 0; k < routetable.size(); k++) {
									if (routetable[k].addr.name == hostAddrs[j].name) {
										bool is1 = false;
										//检测路由转发表中是否存在到中间点z的转发项，若有则更新转发表，将i到j的下一跳路由器定为i到z的下一跳路由器
										for (int m = 0; m < routetable.size(); m++) {
											if (routetable[m].addr.name == hostAddrs[z].name) {
												strcpy(routetable[k].nexthop, routetable[m].nexthop);
												routetable[k].cost = myTable[i][j];
												is1 = true;
												break;
											}
										}
										//不存在到中间点z的转发项，则i到j的下一跳就为z
										if (!is1) {
											strcpy(routetable[k].nexthop, hostAddrs[z].ipaddress);
											routetable[k].cost = myTable[i][j];
										}
										is = true;
										break;
									}
								}
								if (!is) {
									//寻找本机到中间路由z的下一跳
									for (int m = 0; m < routetable.size(); m++) {
										//将i到j的下一跳变为i到z的下一跳
										if (routetable[m].addr.name == hostAddrs[z].name) {
											Addr destAddr = Addr(hostAddrs[j]);
											char nextRoute[16];
											strcpy(nextRoute, routetable[m].nexthop);
											routetable.push_back(routeTableEntry(destAddr, nextRoute, myTable[i][j]));
										}
									}
								}
							}
						}
					}
				}
			}
		}

		vector<routeTableEntry>::iterator iter;
		for (iter = routetable.begin(); iter != routetable.end(); ) {
			if (iter->addr.name == anHostName && myTable[myHostName - 'A'][anHostName - 'A'] >= 9999) {
				routetable.erase(iter);
				break;
			}
			else {
				iter++;
			}
		}
		for (int i = 0; i < Dis_vector.size(); i++) {
			Dis_vector[i] = myTable[myHostName - 'A'][i];
		}
		if (origin_table == myTable) {
			return false;
		}
		else {
			return true;
		}

	}

	char* getNextHop(Addr dst) {   // 获得下一跳路由器
	    dst.name = getHostName(dst.ipaddress);
		for (int i = 0; i < routetable.size(); i++) {
			if (routetable[i].addr.name == dst.name) return routetable[i].nexthop;
		}
		return ("0.0.0.0");//默认返回
	}

	void print() {
	    cout << "-------------------------" << endl;
		for (auto v : routetable) {
			cout << "destName: " << v.addr.name << endl;
			cout << "NExtIP: " << v.nexthop << endl;
			cout << "Cost: " << v.cost << endl;
		}

		cout << "-------------------------" << endl;
	}

	void copy(vector<vector<int>> &v1, vector<vector<int>> &v2) {
		for (int i = 0; i < v2.size(); i++) {
			vector<int> temp;
			for (int j = 0; j < v1.size(); j++) {
				temp.push_back(v2[i][j]);
			}
			v1.push_back(temp);
		}
	}

	vector<int> get_my_dis_vector(char dis) {//发给dis,处理毒性逆转的问题
		vector<int> temp = Dis_vector;
		for (int j = 0; j < routetable.size(); j++) {
			if (routetable[j].nexthop == hostAddrs[dis - 'A'].ipaddress) {
				temp[routetable[j].addr.name - 'A'] = 9999;
			}
		}
		return temp;
	}

		// 从IP到主机名的转换
	char getHostName(char *ip) {
        for (auto addr : hostAddrs) {
            if (strcmp(addr.ipaddress, ip) == 0)
                return addr.name;
        }
	}
    vector<Addr> getNeighbors() {
		vector<Addr> my_neighbors;
		for (int i = 0; i < networkGraph[myHostName].size(); i++) {
			my_neighbors.push_back(networkGraph[myHostName][i].addr2);
		}
		return my_neighbors;
	}

	bool setdown(char a) {
        vector<vector<int>> origin_table;
        copy(origin_table, myTable);
        myTable[myHostName-'A'][a - 'A'] = 9999;
        for (int i = 0; i < hostAddrs.size(); i++) {
            if (i != a - 'A')
                myTable[a - 'A'][i] = 9999;
            else
                myTable[a - 'A'][i] = 0;
        }
        vector<int> new_nei_dis = vector<int>(hostAddrs.size(), 9999);
        new_nei_dis[a - 'A'] = 0;
        vector<pathInfo>::iterator iter;
        for (iter = networkGraph[myHostName - 'A'].begin(); iter != networkGraph[myHostName - 'A'].end(); iter++) {
            if (iter->addr2.name == a) {
                iter->cost = 9999;
                break;
            }
        }
        vector <routeTableEntry>().swap(routetable);
        DValgorithm(new_nei_dis, a);
        if (origin_table == myTable) {
            return false;
        }
        else {
            return true;
        }
	}

};
	/*
	int main() {
	cout << "hello world" << endl;
	RouteTableDV routeA = RouteTableDV('A');
	RouteTableDV routeB = RouteTableDV('B');
	RouteTableDV routeC = RouteTableDV('C');
	RouteTableDV routeD = RouteTableDV('D');
	RouteTableDV routeE = RouteTableDV('E');
	routeA.DValgorithm(routeC.Dis_vector, 'C');
	routeA.DValgorithm(routeD.Dis_vector, 'D');
	routeA.DValgorithm(routeE.Dis_vector, 'E');
	routeA.print();
	string s;
	s = to_string(123);
	cout << s << endl;

	cout << endl<<endl;
	routeD.DValgorithm(routeA.Dis_vector, 'A');
	routeD.DValgorithm(routeC.Dis_vector, 'C');
	routeD.DValgorithm(routeB.Dis_vector, 'B');
	routeD.print();
	cout << endl << endl;
	routeA.setdown('C');
	routeA.print();
	cout << endl << endl;
	routeD.setdown('C');
	routeD.print();

	return 0;
	}
	*/
