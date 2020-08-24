
#define MAX_PACKET_LEN		1024

#define DHCP_PKT( pkt )  ((char *)(pkt))
#define DHCP_FIELD( pkt, type, loc ) ( *((type *) (DHCP_PKT(pkt) + loc)) )
#define DHCP_PFIELD( pkt, type, loc ) ((type) (DHCP_PKT(pkt) + loc))

#define DHCP_OP( pkt )      DHCP_FIELD( pkt, BYTE,    0x00 )		//返回8位值
#define DHCP_HTYPE( pkt )   DHCP_FIELD( pkt, BYTE,    0x01 )		//返回8位值
#define DHCP_HLEN( pkt )    DHCP_FIELD( pkt, BYTE,    0x02 )		//返回8位值
#define DHCP_HOPS( pkt )    DHCP_FIELD( pkt, BYTE,    0x03 )		//返回8位值
#define DHCP_XID( pkt )     DHCP_FIELD( pkt, DWORD,   0x04 )		//返回32位值
#define DHCP_SECS( pkt )    DHCP_FIELD( pkt, WORD,   0x08 )			//返回16位值
#define DHCP_FLAGS( pkt )   DHCP_FIELD( pkt, WORD,   0x0a )			//返回16位值
#define DHCP_CIADDR( pkt )  DHCP_FIELD( pkt, DWORD,   0x0c )		//返回32位值
#define DHCP_YIADDR( pkt )  DHCP_FIELD( pkt, DWORD,   0x10 )		//返回32位值
#define DHCP_SIADDR( pkt )  DHCP_FIELD( pkt, DWORD,   0x14 )		//返回32位值
#define DHCP_GIADDR( pkt )  DHCP_FIELD( pkt, DWORD,   0x18 )		//返回32位值
#define DHCP_CHADDR( pkt )  DHCP_PFIELD( pkt, char *, 0x1c )		//返回char*指针,客户端硬件地址
#define DHCP_SNAME( pkt )   DHCP_PFIELD( pkt, char *, 0x2c )		//返回char*指针,服务器名(64字节)
#define DHCP_FILE( pkt )    DHCP_PFIELD( pkt, char *, 0x6c )		//返回char*指针,引导文件名(128字节)

//下面开始为可选项(可变长度)
#define DHCP_COOKIE( pkt )  DHCP_FIELD( pkt, DWORD,   0xec )		//返回32位值，魔法Cookie

#define DHCP_OPTIONS( pkt ) DHCP_PFIELD( pkt, char *,   0xf0 )		//返回char*指针,选项列表
#define DHCP_OPTIONS_LEN( pkt_len ) ((pkt_len) - 0xf0 )				//返回值,选项列表长度(数据包长度-0xf0)

//选项条目 由 3部分组成 1字节的标记(tag)(指出是什么条目),1字节数据长度,实际的
//数据(长度由第2部分指定)
#define DHCP_OPTION_CODE( pkt, offset ) (DHCP_OPTIONS(pkt)[offset])		//返回值
#define DHCP_OPTION_LEN( pkt, offset ) (DHCP_OPTIONS(pkt)[offset+1])	//返回值	
#define DHCP_OPTION_DATA( pkt, offset ) (DHCP_OPTIONS(pkt)+offset+2)	//返回char*指针

#define DHCP_MAGIC_COOKIE            0x63825363			//魔法Cookie

#define DHCP_OP_BOOTREQUEST          1					//由客户端发送					
#define DHCP_OP_BOOTREPLY            2					//由服务端发送		

//下面是选项条目的标记(tag),指出是什么条目
#define DHCP_OPTION_PAD             0
#define DHCP_OPTION_MASK			1
#define DHCP_OPTION_VENDOR          43
#define DHCP_OPTION_MSG_TYPE        53
#define DHCP_OPTION_SERVER_ID       54
#define DHCP_OPTION_PARAM_REQ_LIST  55
#define DHCP_OPTION_MAX_MSG_SIZE    57
#define DHCP_OPTION_CLASS_ID        60
#define DHCP_OPTION_CSA             93
#define DHCP_OPTION_CNDI_TYPE       94
#define DHCP_OPTION_CLIENT_UUID     97
#define DHCP_OPTION_END            255

#define DHCP_OPTION_LEASETIME      0x33
#define DHCP_OPTION_RENEWAL        0x3A
#define DHCP_OPTION_REBINDING      0x3B

//PXE专用标记(tag)
#define PXE_DISCOVERY_CONTROL        6
#define PXE_MCAST_ADDR               7
#define PXE_BOOT_SERVERS             8
#define PXE_BOOT_MENU                9
#define PXE_MENU_PROMPT             10
#define PXE_BOOT_ITEM               71

//下面是包类型(DHCP_OPTION_MSG_TYPE)

#define DHCP_DISCOVER                1
#define DHCP_OFFER                   2
#define DHCP_REQUEST                 3
#define DHCP_DECLINE                 4
#define DHCP_ACK                     5
#define DHCP_NAK                     6
#define DHCP_RELEASE                 7
#define DHCP_INFORM                  8

//选项
struct dhcp_option 
{
	BYTE len;
	char *data;
};

//选项列表
struct dhcp_option_list
{
	int len;			//选项列表的实际长度
	int size;			//存放选项列表的缓冲区的大小
	char *data;			//存放选项列表的缓冲区首地址
};


//不是真正的DHCP包格式,只是方便程序处理而定义
struct dhcp_packet 
{
	BYTE op;
	BYTE htype;
	BYTE hlen;
	BYTE hops;
	DWORD xid;
	WORD secs;
	WORD flags;
	DWORD ciaddr;
	DWORD yiaddr;
	DWORD siaddr;
	DWORD giaddr;
	char chaddr[16];
	char *sname;
	char *file;
	
	struct dhcp_option options[256];			//选项列表		
	struct dhcp_option vendor_options[256];		//代表43标记(实际上43标记就是一个选项列表)
	
	char *data;
	int data_len;
};


BOOL DHCP_ParsePacket(char * pData, int nDateLen, struct dhcp_packet * pPacket);

void DHCP_SetOption(struct dhcp_packet * pPacket, int nOption, BYTE btLen, void * pData);

void Options_Init(struct dhcp_option_list * pList, char *pBuf, int nBufLen);

BOOL Options_Add(struct dhcp_option_list *pList, BYTE btCode, void *pData, BYTE btLen);

BOOL Options_End(struct dhcp_option_list * pList);

int Options_Len(struct dhcp_option_list * pList);

ULONG DHCP_MakeSendData(struct dhcp_packet * pPacket, PCHAR pSendBuf ,ULONG nBufLen);

//======================================================================================

BOOL IPPool_UserIsExist(PCHAR strMac);

BOOL IPPool_AddUser(PCHAR strMac);

BOOL IPPool_GetIPAddress(PCHAR strMac, PCHAR strIP);
