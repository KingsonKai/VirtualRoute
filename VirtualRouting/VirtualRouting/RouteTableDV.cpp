class RouteTableDV
{
public:
    std::vector<routeTableEntry> routetable;

    // 存储网络拓补图的链路信息，char类型为主机的名字（ABCD
    // 其余为链路信息，对于DV算法来讲，只要存下邻居的路由表信息即可
    map<char, std::vector<pathInfo>> networkGraph;

    RouteTableDV();
    ~RouteTableDV();

    void DValgorithm();          // 更新路由表
    void getNextHop(Addr dst);   // 获得下一跳路由器
};