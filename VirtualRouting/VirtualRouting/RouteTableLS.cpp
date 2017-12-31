#include<vector>
#include<iostream>
#include<map>
#include<queue>
#include "DataStructure.cpp"
using namespace std;
class RouteTableLS
{
public:
	vector<routeTableEntry> routetable;
	// 存储网络拓补图的链路信息，char类型为主机的名字ABCD
	map<char, vector<pathInfo>> networkGraph;//整个网络拓扑图的边,networkGraph['A']包括所有从A出发的边
	vector<Addr> hostAddrs;  //方便主机名和ip地址的转换
	char myHostName;//本机主机名
	RouteTableLS(char name) {
		myHostName = name;
		//获取所有hostAddrs,0对应A的信息
		hostAddrs.push_back(Addr('A', "172.18.157.159"));
		hostAddrs.push_back(Addr('B', "172.18.156.76"));//赋值后是多少位
		hostAddrs.push_back(Addr('C', "172.18.159.66"));
		hostAddrs.push_back(Addr('D', "172.18.159.150"));
		hostAddrs.push_back(Addr('E', "172.18.158.165"));
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
		routetable.clear();
		vector<int> dis(hostAddrs.size(), 9999);//各个结点到源的最短距离,9999代表无穷大
		vector<int> pre(hostAddrs.size(), -1);//前结点
		vector<bool> isVisit(hostAddrs.size(), false);
		queue<char> q;
		q.push(myHostName);
		dis[myHostName - 'A'] = 0;
		pre[myHostName - 'A'] = myHostName - 'A';
		while (!q.empty()) {
			char temp = q.front();
			q.pop();
			for (int i = 0; i < networkGraph[temp].size(); i++) {
				char add2name = networkGraph[temp][i].addr2.name;
				if (!isVisit[add2name - 'A']) q.push(add2name);
				if (networkGraph[temp][i].cost + dis[temp - 'A'] < dis[add2name - 'A']) {
					dis[add2name - 'A'] = networkGraph[temp][i].cost + dis[temp - 'A'];
					pre[add2name - 'A'] = temp - 'A';
				}
			}
			isVisit[temp-'A'] = true;
		}
		for (int i = 0; i < hostAddrs.size(); i++) {//是否需要整条路径信息,应该不需要
			int getStart = hostAddrs[i].name - 'A';
			while (pre[getStart] != myHostName - 'A') {//获取下一跳的主机
				getStart = pre[getStart];
			}
			routetable.push_back(routeTableEntry(hostAddrs[i], hostAddrs[getStart].ipaddress, dis[hostAddrs[i].name - 'A']));
		}
	}
	char* getNextHop(Addr dst) {   // 获得下一跳路由器
		for (int i = 0; i < routetable.size(); i++) {
			if (routetable[i].addr.name == dst.name) return routetable[i].nexthop;
		}
		return ("0.0.0.0");//默认返回
	}
};


