class RouteTableLS
{
public:
    std::vector<routeTableEntry> routetable;

    // 存储网络拓补图的链路信息，char类型为主机的名字（ABCD
    // 其余为链路信息，对于LS算法来讲，要存下整个网络拓补图
    map<char, std::vector<pathInfo>> networkGraph;

    RouteTableLS();
    ~RouteTableLS();

    void LSalgorithm();          // 更新路由表
    void getNextHop(Addr dst);   // 获得下一跳路由器
};