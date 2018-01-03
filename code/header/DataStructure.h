#define SIZE 16
// �����ĵ�ַ��Ϣ
class Addr {
public:
    char name;                 // ��ABCD
    char ipaddress[SIZE];      // IP��ַ
    Addr(char name, char *ip);

    Addr();

	bool operator==(Addr other);
};

// ·�ɱ���Ϣ��λ
class routeTableEntry {
public:
    Addr addr;
    char nexthop[SIZE];         // ��һ��·����
    int cost;                   // ·������

    routeTableEntry(Addr addr, char nexthop[16], int cost);
};

ostream& operator<<(ostream& out, routeTableEntry &e);

// �����ز�ͼ�е�·����Ϣ
class pathInfo {
public:
    Addr addr1, addr2;
    int cost;

	pathInfo(Addr addr1, Addr addr2, int cost);
};

ostream& operator<<(ostream& out, pathInfo &p);

// ·������Ϣ
class route {
public:
    Addr addr;
    int cost;                    // ��·�����Ŀ���
    bool isNeighbor;             // �Ƿ����ھ�
    bool dumped;                 // �Ƿ��Ѿ�down��
    int lastBeatTime;

    route(Addr addr, int cost = -1, bool isNeighbor = false, bool dumped = false);

    bool isDumped();

    void setDumped();
};

ostream& operator<<(ostream& out, route &r);
