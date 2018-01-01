#include<vector>
#include<iostream>
#include<map>
#include<queue>
using namespace std;
class RouteTableLS
{
public:
	vector<routeTableEntry> routetable;

	map<char, vector<pathInfo>> networkGraph;
	vector<Addr> hostAddrs;
	char myHostName;

	RouteTableLS(char name='A');
	~RouteTableLS();

	void LSalgorithm();
	char* getNextHop(Addr dst);
	char getRouterName(char *ip);
};


