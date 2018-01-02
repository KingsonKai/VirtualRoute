#define SIZE 16
// 基本的地址信息
class Addr {
public:
    char name;                 // 如ABCD
    char ipaddress[SIZE];      // IP地址
    Addr(char name, char *ip) {
        this->name = name;
        strcpy(ipaddress, ip);
    }

    Addr() {
        name = '1';
    }

	bool operator==(Addr other) {
		if (name == other.name && strcmp(ipaddress, other.ipaddress)) {
			return true;
		}
		else {
			return false;
		}
	}
};

// 路由表信息单位
class routeTableEntry {
public:
    Addr addr;
    char nexthop[SIZE];         // 下一跳路由器
    int cost;                   // 路径开销

    routeTableEntry(Addr addr, char nexthop[16], int cost) {
        this->addr = addr;
        strcpy(this->nexthop, nexthop);
        this->cost = cost;
    }
};

ostream& operator<<(ostream& out, routeTableEntry &e) {
    cout << "Destination address: " << e.addr.ipaddress << " | " << "NextHop: " << e.nexthop << " | " << "Cost: " << e.cost << endl;
    return out;
}

// 网络拓补图中的路径信息
class pathInfo {
public:
    Addr addr1, addr2;
    int cost;

	pathInfo(Addr addr1, Addr addr2, int cost) {
        this->addr1 = addr1;
        this->addr2 = addr2;
        this->cost = cost;
    }
};

ostream& operator<<(ostream& out, pathInfo &p) {
    cout << "Address 1: " << p.addr1.ipaddress << " | " << "Address 2: " << p.addr2.ipaddress << " | " << "Cost: " << p.cost << endl;
    return out;
}

// 路由器信息
class route {
public:
    Addr addr;
    int cost;                    // 到路由器的开销
    bool isNeighbor;             // 是否是邻居
    bool dumped;                 // 是否已经down掉

    route(Addr addr, int cost = -1, bool isNeighbor = false, bool dumped = false) {
        this->addr = addr;
        this->cost = cost;
        this->isNeighbor = isNeighbor;
        this->dumped = dumped;
    }

    bool isDumped() {
        return dumped;
    }

    void setDumped() {
        this->dumped = true;
        this->cost = -1;
    }
};

ostream& operator<<(ostream& out, route &r) {
    cout << "Router address: " << r.addr.ipaddress << " | " << "Cost: " << r.cost << " | " << "Dumped or not?" << (r.dumped == false ? "No" : "Yes");
    return out;
}
