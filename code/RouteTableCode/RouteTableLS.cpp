#include<vector>
#include<iostream>
#include<map>
#include<queue>
#include "../DataStructure.cpp"
#include<iterator>
using namespace std;

class RouteTableLS {
public:
	vector<routeTableEntry> routetable;
	// 存储网络拓补图的链路信息，char类型为主机的名字ABCD
	map<char, vector<pathInfo>> networkGraph;//整个网络拓扑图的边,networkGraph['A']包括所有从A出发的边
	vector<Addr> hostAddrs;  //方便主机名和ip地址的转换
	char myHostName;//本机主机名
	vector<char> have_delete;
	RouteTableLS(char name = 'D') {
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

		//获取整体拓扑结构,暂时手动输入获取,后期可以尝试动态修改，就是创建了UI之后启动程序让用户输入边数和边权
		networkGraph['A'] = vector<pathInfo>{
			pathInfo(hostAddrs[0], hostAddrs['C' - 'A'], 3),
			pathInfo(hostAddrs[0], hostAddrs['D' - 'A'], 7),
			pathInfo(hostAddrs[0], hostAddrs['E' - 'A'], 5)
		};
		networkGraph['B'] = vector<pathInfo>{
			pathInfo(hostAddrs[1], hostAddrs['D' - 'A'], 1),
			pathInfo(hostAddrs[1], hostAddrs['E' - 'A'], 3)
		};
		networkGraph['C'] = vector<pathInfo>{
			pathInfo(hostAddrs[2], hostAddrs[0], 3),
			pathInfo(hostAddrs[2], hostAddrs['D' - 'A'], 2)
		};
		networkGraph['D'] = vector<pathInfo>{
			pathInfo(hostAddrs[3], hostAddrs[0], 7),
			pathInfo(hostAddrs[3], hostAddrs['B' - 'A'], 1),
			pathInfo(hostAddrs[3], hostAddrs['C' - 'A'], 2)
		};
		networkGraph['E'] = vector<pathInfo>{
			pathInfo(hostAddrs[4], hostAddrs[0], 5),
			pathInfo(hostAddrs[4], hostAddrs['B' - 'A'], 3)
		};
		LSalgorithm();//更新路由表
	}
	~RouteTableLS() {}

	void LSalgorithm() {// 更新路由表，01234对应ABCDE,已经在构造函数中调用了，不需要线程中再次调用，DV才要调用
		//routetable.clear();
		vector<int> dis(hostAddrs.size(), 9999);//各个结点到源的最短距离,9999代表无穷大
		vector<int> pre(hostAddrs.size(), -1);//前结点

		vector<bool> isVisit(hostAddrs.size(), false);

		queue<char> q;
		q.push(myHostName);
		dis[myHostName - 'A'-index_sub(have_delete,myHostName)] = 0;
		pre[myHostName - 'A' - index_sub(have_delete, myHostName)] = myHostName - 'A' - index_sub(have_delete, myHostName);


		while (!q.empty()) {
			char temp = q.front();
			q.pop();
			for (int i = 0; i < networkGraph[temp].size(); i++) {
				char add2name = networkGraph[temp][i].addr2.name;
				if (!isVisit[add2name - 'A'- index_sub(have_delete, add2name)]) q.push(add2name);
				if (networkGraph[temp][i].cost + dis[temp - 'A'- index_sub(have_delete, temp)] < dis[add2name - 'A'- index_sub(have_delete, add2name)]) {
					dis[add2name - 'A'- index_sub(have_delete, add2name)] = networkGraph[temp][i].cost + dis[temp - 'A' - index_sub(have_delete, temp)];
					pre[add2name - 'A'- index_sub(have_delete, add2name)] = temp - 'A' - index_sub(have_delete, temp);
				}
			}
			isVisit[temp - 'A' - index_sub(have_delete, temp)] = true;
		}

		for (int i = 0; i < hostAddrs.size(); i++) {//是否需要整条路径信息,应该不需要
			int getStart = hostAddrs[i].name - 'A'-index_sub(have_delete, hostAddrs[i].name);
			while (pre[getStart] != myHostName - 'A'-index_sub(have_delete, myHostName)) {//获取下一跳的主机
				getStart = pre[getStart];
			}
			routetable.push_back(routeTableEntry(hostAddrs[i], hostAddrs[getStart].ipaddress, dis[hostAddrs[i].name - 'A'- index_sub(have_delete, hostAddrs[i].name)]));
		}
	}

	char* getNextHop(Addr dst) {   // 获得下一跳路由器
		dst.name = getHostName(dst.ipaddress);
		for (int i = 0; i < routetable.size(); i++) {
			if (routetable[i].addr.name == dst.name) return routetable[i].nexthop;
		}
		return ("0.0.0.0");//默认返回
	}

	bool setDown(Addr a) {
		bool is = false;
		for (int i = 0; i < hostAddrs.size(); i++) {
			if (hostAddrs[i] == a) {
				have_delete.push_back(a.name);
				is = true;
				break;
			}
		}

		//当is为false的时候，代表已经down掉了a这个主机，所以直接返回不需要修改即可，当为true的时候，代表需要down调主机a并且更新路由表
		if (is == false) return is;
		//将路由表清空
		vector <routeTableEntry>().swap(routetable);

		vector<Addr>::iterator it;
		//在地址的映射中去掉主机a

		for (it = hostAddrs.begin(); it != hostAddrs.end(); it++) {
			//cout << it->name << " " << a.name << endl;
			if (*it == a) {
				hostAddrs.erase(it);
				break;
			}
		}

		//cout << hostAddrs.size() << endl;
		//
		map<char, vector<pathInfo>>::iterator iter_delete = networkGraph.find(a.name);
		map<char, vector<pathInfo>>::iterator iter;
		networkGraph.erase(iter_delete);
		int m = 0;
		for (iter = networkGraph.begin(); iter != networkGraph.end(); iter++) {

			vector<pathInfo>::iterator iter1;
			for (iter1 = iter->second.begin(); iter1 != iter->second.end(); ) {
				//cout << iter1->addr1.name << " " << iter1->addr2.name << endl;
				if (iter1->addr2 == a) {
					iter1 = iter->second.erase(iter1);
					break;
				}
				else {
					iter1++;
				}
			}
		}

		LSalgorithm();

		return is;
	}

	vector<Addr> getNeighbors() {
		vector<Addr> my_neighbors;
		for (int i = 0; i < networkGraph[myHostName].size(); i++) {
			my_neighbors.push_back(networkGraph[myHostName][i].addr2);
		}
		return my_neighbors;
	}

	// 主机名到IP地址的转换
	char* getIPAddress(char name) {
		for (auto addr : hostAddrs) {
			if (name == addr.name) {
				return addr.ipaddress;
			}
		}
		return NULL;
	}

	// 从IP到主机名的转换
	char getHostName(char *ip) {
		for (auto addr : hostAddrs) {
			if (strcmp(addr.ipaddress, ip) == 0)
				return addr.name;
		}
	}
	void print() {
		for (auto a : routetable) {
			cout << "目的地址：" << a.addr.name << endl;
			cout << "下一跳路由器：" << a.nexthop << endl;
			cout << "代价：" << a.cost << endl;
		}
		cout << "-----------------------------------" << endl;
	}
	int index_sub(vector<char> have_delete, char a) {
		int dis = 0;
		for (int i = 0; i < have_delete.size(); i++) {
			if (a > have_delete[i]) {
				dis++;
			}
		}
		return dis;
	}
};

