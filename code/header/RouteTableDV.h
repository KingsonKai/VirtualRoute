#include<vector>
#include<iostream>
#include<map>
#include<queue>
#include "DataStructure.cpp"
using namespace std;
class RouteTableDV
{
public:
	vector<routeTableEntry> routetable;
	vector<vector<int>> myTable;
	map<char, vector<pathInfo>> networkGraph;
	vector<Addr> hostAddrs;
	char myHostName;


	RouteTableDV(char name);
	~RouteTableDV();

	void DValgorithm(vector<vector<int>> anTable, char anHostName);
	char* getNextHop(Addr dst);
	void print();
};
