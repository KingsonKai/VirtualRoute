class virtualPacket
{
public:
    int type;
    Addr source;
    Addr dst;
    char message[MAX_BYTE];
    virtualPacket() {
    }
    ~virtualPacket();

    // 所有要发送的信息，构建好一条字符串，然后复制到sendMessage中去
    // sendMessage是指针，复制后内容会改变，在路由器模块中就能接收到消息

    // 根据要发送的message，构建好字符流，调用strncpy复制到sendMessage中
    void constructNormalPacket(char *sendMessage, char *message);

    // 打包路由表信息
    void constructRouterInfoPacket(char *sendMessage, std::vector<routeTableEntry> routeTable);

    // 打包心跳检测信息
    void constructHeartBeatPecket(char *sendMessage, char *dst);

    // 根据接收到的信息，实例化为一个virtual packet
    void makePacket(char *receivedMessage);

    // 输出这个包的内容
    void print();
};