
#include "stdafx.h"
#include "common.h"

//=================================================================================
#define dwItemCount 250

typedef struct tagIP_POOL_ITEM
{
	CHAR strIP[32];
	CHAR strMac[32];
} IP_POOL_ITEM, *PIP_POOL_ITEM;


typedef struct tagIP_POOL
{
	IP_POOL_ITEM Item[dwItemCount];
} IP_POOL, *PIP_POOL;


static CRITICAL_SECTION g_csPoolLock;
static IP_POOL          g_IPPool  = {0};
static ULONG            uFourNode = 2;

//IP池初始化
void IPPool_Init()
{
	InitializeCriticalSection(&g_csPoolLock);
}

//IP池释放
void IPPool_Free()
{
	DeleteCriticalSection(&g_csPoolLock);
}

//指定的MAC所代表的纪录是不是已经存在
BOOL IPPool_UserIsExist(PCHAR strMac)
{
	BOOL bResult = FALSE;
	EnterCriticalSection(&g_csPoolLock);
	for(ULONG i = 0; i < dwItemCount; i++)
	{
		PIP_POOL_ITEM Item;
		Item = &g_IPPool.Item[i];
		if (stricmp(Item->strMac, strMac) == 0)
		{
			bResult = TRUE;
			break;
		}
	}
	LeaveCriticalSection(&g_csPoolLock);
	return bResult;
}

//在IP池中新分配一条纪录
BOOL IPPool_AddUser(PCHAR strMac)
{
	BOOL bResult = FALSE;
	EnterCriticalSection(&g_csPoolLock);
	PIP_POOL_ITEM Item;	
	for(ULONG i = 0; i < dwItemCount; i++)
	{
		Item = &g_IPPool.Item[i];
		if (strlen(Item->strMac) == 0)
		{
			bResult = TRUE;
			break;
		}
	}
	if (bResult)
	{
		CHAR strIP[32];
		wsprintf(strIP, "192.168.88.%u", uFourNode);
		uFourNode++;
		strcpy(Item->strIP , strIP);
		strcpy(Item->strMac, strMac);
	}
	LeaveCriticalSection(&g_csPoolLock);
	return bResult;
}


BOOL IPPool_GetIPAddress(PCHAR strMac, PCHAR strIP)
{
	BOOL bResult = FALSE;
	EnterCriticalSection(&g_csPoolLock);
	for(ULONG i = 0; i < dwItemCount; i++)
	{
		PIP_POOL_ITEM Item;
		Item = &g_IPPool.Item[i];
		if (stricmp(Item->strMac, strMac) == 0)
		{
			strcpy(strIP, Item->strIP);
			bResult = TRUE;
			break;
		}
	}
	LeaveCriticalSection(&g_csPoolLock);
	return bResult;
}

//=================================================================================

/*
数据包的首地址(IN)
数据包的长度(IN)
dhcp_packet结构首地址(OUT)
作用:解析收到的数据包，填充dhcp_packet结构
*/
BOOL DHCP_ParsePacket(char * pData, int nDateLen, struct dhcp_packet * pPacket)
{
	//包的长度太小
	if (nDateLen < 272)
		return FALSE;

	// 清理字符串
	DHCP_SNAME(pData)[63] = 0;
	DHCP_FILE(pData)[127] = 0;

	memset(pPacket, 0, sizeof(*pPacket));

	pPacket->op     = DHCP_OP(pData);
	pPacket->htype  = DHCP_HTYPE(pData);
	pPacket->hlen   = DHCP_HLEN(pData);
	pPacket->hops   = DHCP_HOPS(pData);
	pPacket->xid    = ntohl(DHCP_XID(pData));
	pPacket->secs   = ntohs(DHCP_SECS(pData));
	pPacket->flags  = ntohs(DHCP_FLAGS(pData));
	pPacket->ciaddr = ntohl(DHCP_CIADDR(pData));
	pPacket->yiaddr = ntohl(DHCP_YIADDR(pData));
	pPacket->siaddr = ntohl(DHCP_SIADDR(pData));
	pPacket->giaddr = ntohl(DHCP_GIADDR(pData));
	memcpy(pPacket->chaddr, DHCP_CHADDR(pData), 16);
	pPacket->sname  = DHCP_SNAME(pData);
	pPacket->file   = DHCP_FILE(pData);
	
	for(int i=0; i<DHCP_OPTIONS_LEN(nDateLen); i++)
	{
		if(DHCP_OPTION_CODE(pData, i) == DHCP_OPTION_PAD) 
			continue;
		else if(DHCP_OPTION_CODE(pData, i) == DHCP_OPTION_END)
			break;

		pPacket->options[DHCP_OPTION_CODE(pData, i)].len  = DHCP_OPTION_LEN(pData, i);
		pPacket->options[DHCP_OPTION_CODE(pData, i)].data = DHCP_OPTION_DATA(pData, i);
		
		if(DHCP_OPTION_CODE(pData, i) == DHCP_OPTION_VENDOR) 
		{
			for(int j=0; j < DHCP_OPTION_LEN(pData, i); j++) 
			{
				if(DHCP_OPTION_CODE(pData, i+j+2) == DHCP_OPTION_PAD) 
				{
					continue;
				}
				if(DHCP_OPTION_CODE(pData, i+j+2) == DHCP_OPTION_END)
				{
					break;
				}
				
				pPacket->vendor_options[DHCP_OPTION_CODE(pData, i+j+2)].len  = DHCP_OPTION_LEN(pData, i+j+2);
				pPacket->vendor_options[DHCP_OPTION_CODE(pData, i+j+2)].data = DHCP_OPTION_DATA(pData, i+j+2);   
			
				//这句难到不要加吗?
				j += DHCP_OPTION_LEN(pData, (i+2)+j) + 1;
			}
		}
		
		i += DHCP_OPTION_LEN(pData, i) + 1;
	}

	//我是服务端,我只接收从客户端传给服务端的数据包
	if (pPacket->op != DHCP_OP_BOOTREQUEST)
		return FALSE;

	//不是一个有效的DHCP包
	if(pPacket->options[DHCP_OPTION_MSG_TYPE].len == 0)
		return FALSE;	

	//不包含有60标记，这不是一个PXE客户端发过来的包
//	if(pPacket->options[DHCP_OPTION_CLASS_ID].len == 0)
//		return FALSE;

	return TRUE;

}


void DHCP_SetOption(struct dhcp_packet * pPacket, int nOption, BYTE btLen, void * pData)
{
	pPacket->options[nOption].len  = btLen;
	pPacket->options[nOption].data = (char *) pData;
}


void Options_Init(struct dhcp_option_list * pList, char *pBuf, int nBufLen)
{
	pList->len  = 0;				//实际长度
	pList->data = pBuf;			//存放数据的缓冲区大小
	pList->size = nBufLen;		//缓冲区首地址
}


BOOL Options_Add(struct dhcp_option_list *pList, BYTE btCode, void *pData, BYTE btLen)
{
	//加2是因为有1字节的code,1字节的len
	//数据区也是一个选项列表,但是最大长度只能是255
	if((pList->len+btLen+2) > pList->size) 
		return FALSE;
	pList->data[pList->len]     = btCode;
	pList->data[pList->len + 1] = btLen;
	memcpy(pList->data + pList->len + 2, pData, btLen);
	pList->len += btLen + 2;
	return TRUE;
}


BOOL Options_End(struct dhcp_option_list * pList)
{
	if((pList->len + 1) > pList->size) 
		return FALSE;
	pList->data[pList->len] = (char)255;
	pList->len++;
	return TRUE;
}


int Options_Len(struct dhcp_option_list * pList)
{
	return pList->len;
}


/*
dhcp_packet结构首地址(IN)
缓冲区首地址(OUT)
缓冲区长度(IN)
返回值:真正的dhcp数据包的长度
作用:将dhcp_packet结构解析成真正的dhcp数据格式存放在缓冲区中
*/
ULONG DHCP_MakeSendData(struct dhcp_packet * pPacket, PCHAR pSendBuf, ULONG nBufLen)
{
	memset(pSendBuf, 0, nBufLen);
	
	DHCP_OP(pSendBuf)     = pPacket->op;
	DHCP_HTYPE(pSendBuf)  = pPacket->htype;
	DHCP_HLEN(pSendBuf)   = pPacket->hlen;
	DHCP_HOPS(pSendBuf)   = pPacket->hops;
	DHCP_XID(pSendBuf)    = htonl(pPacket->xid);
	DHCP_SECS(pSendBuf)   = htons(pPacket->secs);
	DHCP_FLAGS(pSendBuf)  = htons(pPacket->flags);
	DHCP_CIADDR(pSendBuf) = htonl(pPacket->ciaddr);
	DHCP_YIADDR(pSendBuf) = htonl(pPacket->yiaddr);
	DHCP_SIADDR(pSendBuf) = htonl(pPacket->siaddr);
	DHCP_GIADDR(pSendBuf) = htonl(pPacket->giaddr);
	memcpy(DHCP_CHADDR(pSendBuf), pPacket->chaddr, 16);
	if(pPacket->sname)
		memcpy(DHCP_SNAME(pSendBuf), pPacket->sname, 64);
	if(pPacket->file)
		memcpy(DHCP_FILE(pSendBuf), pPacket->file, 128);

	DHCP_COOKIE(pSendBuf) = htonl(DHCP_MAGIC_COOKIE);

	struct dhcp_option_list stOptList;

	Options_Init(&stOptList, DHCP_OPTIONS(pSendBuf), DHCP_OPTIONS_LEN(MAX_PACKET_LEN));
	
	for(int i=0; i<256; i++) 
	{
		if(pPacket->options[i].len != 0)
		{
			if(!Options_Add(&stOptList, i, pPacket->options[i].data, pPacket->options[i].len))
			{
				return 0;	//出错
			}
		}
	}
	
	if(!Options_End(&stOptList))
	{
		return 0;	//出错	
	}

	return Options_Len(&stOptList) + 0xf0;
}

//===========================================================
extern CRITICAL_SECTION g_csDhcpLock;
extern CRITICAL_SECTION g_csPxeLock;

void StartupServiceInit()
{
	IPPool_Init();
	InitializeCriticalSection(&g_csDhcpLock);
	InitializeCriticalSection(&g_csPxeLock);
}

void StartupServiceFree()
{
	IPPool_Free();
	DeleteCriticalSection(&g_csDhcpLock);
	DeleteCriticalSection(&g_csPxeLock);		
}
