
#define MAX_PACKET_LEN		1024

#define DHCP_PKT( pkt )  ((char *)(pkt))
#define DHCP_FIELD( pkt, type, loc ) ( *((type *) (DHCP_PKT(pkt) + loc)) )
#define DHCP_PFIELD( pkt, type, loc ) ((type) (DHCP_PKT(pkt) + loc))

#define DHCP_OP( pkt )      DHCP_FIELD( pkt, BYTE,    0x00 )		//����8λֵ
#define DHCP_HTYPE( pkt )   DHCP_FIELD( pkt, BYTE,    0x01 )		//����8λֵ
#define DHCP_HLEN( pkt )    DHCP_FIELD( pkt, BYTE,    0x02 )		//����8λֵ
#define DHCP_HOPS( pkt )    DHCP_FIELD( pkt, BYTE,    0x03 )		//����8λֵ
#define DHCP_XID( pkt )     DHCP_FIELD( pkt, DWORD,   0x04 )		//����32λֵ
#define DHCP_SECS( pkt )    DHCP_FIELD( pkt, WORD,   0x08 )			//����16λֵ
#define DHCP_FLAGS( pkt )   DHCP_FIELD( pkt, WORD,   0x0a )			//����16λֵ
#define DHCP_CIADDR( pkt )  DHCP_FIELD( pkt, DWORD,   0x0c )		//����32λֵ
#define DHCP_YIADDR( pkt )  DHCP_FIELD( pkt, DWORD,   0x10 )		//����32λֵ
#define DHCP_SIADDR( pkt )  DHCP_FIELD( pkt, DWORD,   0x14 )		//����32λֵ
#define DHCP_GIADDR( pkt )  DHCP_FIELD( pkt, DWORD,   0x18 )		//����32λֵ
#define DHCP_CHADDR( pkt )  DHCP_PFIELD( pkt, char *, 0x1c )		//����char*ָ��,�ͻ���Ӳ����ַ
#define DHCP_SNAME( pkt )   DHCP_PFIELD( pkt, char *, 0x2c )		//����char*ָ��,��������(64�ֽ�)
#define DHCP_FILE( pkt )    DHCP_PFIELD( pkt, char *, 0x6c )		//����char*ָ��,�����ļ���(128�ֽ�)

//���濪ʼΪ��ѡ��(�ɱ䳤��)
#define DHCP_COOKIE( pkt )  DHCP_FIELD( pkt, DWORD,   0xec )		//����32λֵ��ħ��Cookie

#define DHCP_OPTIONS( pkt ) DHCP_PFIELD( pkt, char *,   0xf0 )		//����char*ָ��,ѡ���б�
#define DHCP_OPTIONS_LEN( pkt_len ) ((pkt_len) - 0xf0 )				//����ֵ,ѡ���б���(���ݰ�����-0xf0)

//ѡ����Ŀ �� 3������� 1�ֽڵı��(tag)(ָ����ʲô��Ŀ),1�ֽ����ݳ���,ʵ�ʵ�
//����(�����ɵ�2����ָ��)
#define DHCP_OPTION_CODE( pkt, offset ) (DHCP_OPTIONS(pkt)[offset])		//����ֵ
#define DHCP_OPTION_LEN( pkt, offset ) (DHCP_OPTIONS(pkt)[offset+1])	//����ֵ	
#define DHCP_OPTION_DATA( pkt, offset ) (DHCP_OPTIONS(pkt)+offset+2)	//����char*ָ��

#define DHCP_MAGIC_COOKIE            0x63825363			//ħ��Cookie

#define DHCP_OP_BOOTREQUEST          1					//�ɿͻ��˷���					
#define DHCP_OP_BOOTREPLY            2					//�ɷ���˷���		

//������ѡ����Ŀ�ı��(tag),ָ����ʲô��Ŀ
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

//PXEר�ñ��(tag)
#define PXE_DISCOVERY_CONTROL        6
#define PXE_MCAST_ADDR               7
#define PXE_BOOT_SERVERS             8
#define PXE_BOOT_MENU                9
#define PXE_MENU_PROMPT             10
#define PXE_BOOT_ITEM               71

//�����ǰ�����(DHCP_OPTION_MSG_TYPE)

#define DHCP_DISCOVER                1
#define DHCP_OFFER                   2
#define DHCP_REQUEST                 3
#define DHCP_DECLINE                 4
#define DHCP_ACK                     5
#define DHCP_NAK                     6
#define DHCP_RELEASE                 7
#define DHCP_INFORM                  8

//ѡ��
struct dhcp_option 
{
	BYTE len;
	char *data;
};

//ѡ���б�
struct dhcp_option_list
{
	int len;			//ѡ���б��ʵ�ʳ���
	int size;			//���ѡ���б�Ļ������Ĵ�С
	char *data;			//���ѡ���б�Ļ������׵�ַ
};


//����������DHCP����ʽ,ֻ�Ƿ�������������
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
	
	struct dhcp_option options[256];			//ѡ���б�		
	struct dhcp_option vendor_options[256];		//����43���(ʵ����43��Ǿ���һ��ѡ���б�)
	
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
