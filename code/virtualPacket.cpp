class virtualPacket
{
public:
	int type;
	Addr source;
	Addr dst;
	char message[MAXBYTE];
	char recvBuf[MAXBYTE];

	virtualPacket(int type, Addr source, Addr dst, char *message) {
		this->type = type;
		this->source = source;
		this->dst = dst;
		if (message != NULL)
            strcpy(this->message, message);
	}

	virtualPacket() {
		type = -1;
	}

	// 所有要发送的信息，构建好一条字符串，然后复制到sendMessage中去
	// sendMessage是指针，复制后内容会改变，在路由器模块中就能接收到消息

	// 根据要发送的message，构建好字符流，调用strncpy复制到sendMessage中
	void constructNormalPacket(char *sendMessage, char* content) {
	    memset(sendMessage, 0, sizeof(sendMessage));
		sendMessage[0] = '0';
		writeAddress(sendMessage);
		strcat(sendMessage, content);
	}

	// LS
	// 打包down包信息
	void constructDownPacket(char *sendMessage) {
	    memset(sendMessage, 0, sizeof(sendMessage));
		sendMessage[0] = '4';
		writeAddress(sendMessage);
		char content[] = "someone is down!";
		strcat(sendMessage, content);
	}


	/* 打包路由表信息
	将DV算法得到的vector<int>中的数字转换为字符串插入发送的消息中
	代表每个数字的字符串之间用*隔开
	*/
	void constructRouterInfoPacket(char *sendMessage, vector<int> Nei_dis) {
	    memset(sendMessage, 0, sizeof(sendMessage));
		sendMessage[0] = '1';
		writeAddress(sendMessage);
		for (int i = 0; i < Nei_dis.size(); ++i) {
			string num = std::to_string(Nei_dis[i]);
			const char* c = num.data();
			strcat(sendMessage, c);
			strcat(sendMessage, "*");
		}
	}

	/* 解析收到的路由更新包
	提取返回可供DV使用的vector<int>，即距离矢量
	*/
	vector<int> analyzeUpdatePacket() {
		string str_message(message);
		vector<int> num;
		int begin = 0;
		int end = 0;
		int count = 0;
		do {
			if (str_message[end] == '*') {
				string tmpStr = str_message.substr(begin, end - begin);
				int tmpNum = std::atoi(tmpStr.c_str());
				num.push_back(tmpNum);
				begin = end + 1;
				end = begin;
				++count;
			}
			else {
				++end;
			}
		} while (end < str_message.size() && count != 5);

		return num;
	}

	/*
	// 打包路由表信息
	void constructRouterInfoPacket(char *sendMessage, std::vector<routeTableEntry> routeTable) {
        memset(sendMessage, 0, sizeof(sendMessage));
		sendMessage[0] = '1';
		writeAddress(sendMessage + 1);
		char routeMessage[MAXBYTE];
		for (auto e : routeTable) {
			strncpy(routeMessage + strlen(routeMessage), e.addr.ipaddress, strlen(e.addr.ipaddress));
			if (e.cost > 10) {
				sprintf(routeMessage + strlen(routeMessage), "%d", e.cost);
			}
			else {
				routeMessage[strlen(routeMessage)] = '0';
				sprintf(routeMessage + strlen(routeMessage) + 1, "%d", e.cost);
			}
			routeMessage[strlen(routeMessage) + 1] = '*';
		}
		strncpy(sendMessage + 33, routeMessage, strlen(routeMessage));
		sendMessage[33 + strlen(sendMessage)] = '\0';
	}
	*/

	// 打包心跳检测信息
	void constructHeartBeatPacket(char *sendMessage) {
	    memset(sendMessage, 0, sizeof(sendMessage));
		sendMessage[0] = '2';
		writeAddress(sendMessage);
		char content[] = "I am alive!";
		strcat(sendMessage, content);
	}

	// 打包响应包
	void constructResponsePacket(char *sendMessage, char *content) {
	    memset(sendMessage, 0, sizeof(sendMessage));
		sendMessage[0] = '3';
		writeAddress(sendMessage);
		strcat(sendMessage, content);
	}

    void constructRequestPacket(char *sendMessage) {
        memset(sendMessage, 0, sizeof(sendMessage));
        sendMessage[0] = '5';
        writeAddress(sendMessage);
        strcat(sendMessage, message);
    }

	// LS
	// 只改变包的目的IP地址，其他不变
	void changeDstIP(char *newDst) {
		strcpy(dst.ipaddress, newDst);
		strncpy(recvBuf + 17, newDst, strlen(newDst));
	}

	void writeAddress(char *sendMessage) {
        strcat(sendMessage, source.ipaddress);
        strcat(sendMessage, "*");
        strcat(sendMessage, dst.ipaddress);
        strcat(sendMessage, "*");
	}

	// 根据接收到的信息，实例化为一个virtual packet
	void makePacket(char *receivedMessage) {
		strcpy(recvBuf, receivedMessage);

		type = receivedMessage[0] - '0';

		strncpy(source.ipaddress, receivedMessage + 1, 15);
		source.ipaddress[15] = '\0';

		strncpy(dst.ipaddress, receivedMessage + 17, 15);
		dst.ipaddress[15] = '\0';

		strncpy(message, receivedMessage + 33, strlen(receivedMessage) - 33);
		message[strlen(receivedMessage)-33] = '\0';
	}

	// 输出这个包的内容
	void print() {
		cout << "--------Packet content--------" << endl;
		cout << "--type: " << type << endl;
		cout << "--source ip address: " << source.ipaddress << endl;
		cout << "--destination ip address: " << dst.ipaddress << endl;
		cout << "--content: " << message << endl;
	}

	int getType() {
		return type;
	}

	Addr getSource() {
		return source;
	}

	Addr getDst() {
		return dst;
	}

	char* getMessage() {
		return message;
	}

	char* getRecvBuf() {
		return recvBuf;
	}
};
