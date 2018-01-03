class virtualPacket
{
public:
	int type;
	Addr source;
	Addr dst;
	char message[MAXBYTE];
	char recvBuf[MAXBYTE];

	virtualPacket(int type, Addr source, Addr dst, char *content);
	virtualPacket();

	// ����Ҫ���͵���Ϣ��������һ���ַ�����Ȼ���Ƶ�sendMessage��ȥ
	// sendMessage��ָ�룬���ƺ����ݻ�ı䣬��·����ģ���о��ܽ��յ���Ϣ

	// ����Ҫ���͵�message���������ַ���������strncpy���Ƶ�sendMessage��
	void constructNormalPacket(char *sendMessage, char* content);
	// ���·�ɱ���Ϣ
	void constructRouterInfoPacket(char *sendMessage, std::vector<routeTableEntry> routeTable);
	// ������������Ϣ
	void constructHeartBeatPacket(char *sendMessage);
	// �����Ӧ��
	void constructResponsePacket(char *sendMessage, char *content);
	void writeAddress(char *sendMessage);
	// ���ݽ��յ�����Ϣ��ʵ����Ϊһ��virtual packet
	void makePacket(char *receivedMessage);

	// ��������������
	void print();
	int getType();
	Addr getSource();
	Addr getDst();
	char* getMessage();
	char* getRecvBuf();
};
