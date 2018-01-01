class virtualPacket
{
public:
	int type;
	Addr source;
	Addr dst;
	char message[MAXBYTE];
	char recvBuf[MAXBYTE + 33];

	virtualPacket(int type, Addr source, Addr dst, char *message) {
		this->type = type;
		this->source = source;
		this->dst = dst;
		strcpy(this->message, message);
	}

	virtualPacket() {
		type = -1;
	}

	// 所有要发送的信息，构建好一条字符串，然后复制到sendMessage中去
	// sendMessage是指针，复制后内容会改变，在路由器模块中就能接收到消息

	// 根据要发送的message，构建好字符流，调用strncpy复制到sendMessage中
	void constructNormalPacket(char *sendMessage, char* message) {
		sendMessage[0] = '0';
		writeAddress(sendMessage + 1);
		strncpy(sendMessage + 33, message, strlen(message));
		sendMessage[33 + strlen(sendMessage)] = '\0';
	}

	// 打包down包信息
	void constructDownPacket(char *sendMessage) {
		sendMessage[0] = '4';
		writeAddress(sendMessage + 1);
		char message[] = "I am going to down!";
		strncpy(sendMessage + 33, "I am going to down!", strlen(message));
	}

	// 打包路由表信息
	void constructRouterInfoPacket(char *sendMessage, std::vector<routeTableEntry> routeTable) {
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

	// 打包心跳检测信息
	void constructHeartBeatPacket(char *sendMessage) {
		sendMessage[0] = '2';
		writeAddress(sendMessage + 1);
		char message[] = "I am alive!";
		strncpy(sendMessage + 33, "I am alive!", strlen(message));
	}

	// 打包响应包
	void constructResponsePacket(char *sendMessage, char *message) {
		sendMessage[0] = '3';
		writeAddress(sendMessage + 1);
		strncpy(sendMessage + 33, message, strlen(message));
		sendMessage[33 + strlen(sendMessage)] = '\0';
	}

	void changeDstIP(char *newDst) {
		strcpy(dst.ipaddress, newDst);
		strncpy(recvBuf + 17, newDst, strlen(newDst));
	}

	void writeAddress(char *sendMessage) {
		strncpy(sendMessage, source.ipaddress, strlen(source.ipaddress));
		sendMessage[16] = '*';
		strncpy(sendMessage + 17, dst.ipaddress, strlen(dst.ipaddress));
		sendMessage[32] = '*';
	}

	// 根据接收到的信息，实例化为一个virtual packet
	void makePacket(char *receivedMessage) {
		strcpy(recvBuf, receivedMessage);
		type = receivedMessage[0] - '0';
		strncpy(source.ipaddress, receivedMessage + 1, 15);
		strncpy(dst.ipaddress, receivedMessage + 17, 15);
<<<<<<< HEAD
		source.ipaddress[16] = '\0';
		dst.ipaddress[16] = '\0';
		strncpy(message, receivedMessage + 33, strlen(receivedMessage) - 32);
=======
		source.ipaddress[15] = '\0';
		dst.ipaddress[15] = '\0';
		strncpy(message, receivedMessage + 33, strlen(receivedMessage) - 33);
		message[strlen(receivedMessage)-33] = 0x00;
>>>>>>> 5b38df841c6ad4cea232346153979741198126c4
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
