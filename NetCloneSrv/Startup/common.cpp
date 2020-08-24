
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

//IP�س�ʼ��
void IPPool_Init()
{
	InitializeCriticalSection(&g_csPoolLock);
}

//IP���ͷ�
void IPPool_Free()
{
	DeleteCriticalSection(&g_csPoolLock);
}

//ָ����MAC������ļ�¼�ǲ����Ѿ�����
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

//��IP�����·���һ����¼
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
���ݰ����׵�ַ(IN)
���ݰ��ĳ���(IN)
dhcp_packet�ṹ�׵�ַ(OUT)
����:�����յ������ݰ������dhcp_packet�ṹ
*/
BOOL DHCP_ParsePacket(char * pData, int nDateLen, struct dhcp_packet * pPacket)
{
	//���ĳ���̫С
	if (nDateLen < 272)
		return FALSE;

	// �����ַ���
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
			
				//����ѵ���Ҫ����?
				j += DHCP_OPTION_LEN(pData, (i+2)+j) + 1;
			}
		}
		
		i += DHCP_OPTION_LEN(pData, i) + 1;
	}

	//���Ƿ����,��ֻ���մӿͻ��˴�������˵����ݰ�
	if (pPacket->op != DHCP_OP_BOOTREQUEST)
		return FALSE;

	//����һ����Ч��DHCP��
	if(pPacket->options[DHCP_OPTION_MSG_TYPE].len == 0)
		return FALSE;	

	//��������60��ǣ��ⲻ��һ��PXE�ͻ��˷������İ�
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
	pList->len  = 0;				//ʵ�ʳ���
	pList->data = pBuf;			//������ݵĻ�������С
	pList->size = nBufLen;		//�������׵�ַ
}


BOOL Options_Add(struct dhcp_option_list *pList, BYTE btCode, void *pData, BYTE btLen)
{
	//��2����Ϊ��1�ֽڵ�code,1�ֽڵ�len
	//������Ҳ��һ��ѡ���б�,������󳤶�ֻ����255
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
dhcp_packet�ṹ�׵�ַ(IN)
�������׵�ַ(OUT)
����������(IN)
����ֵ:������dhcp���ݰ��ĳ���
����:��dhcp_packet�ṹ������������dhcp���ݸ�ʽ����ڻ�������
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
				return 0;	//����
			}
		}
	}
	
	if(!Options_End(&stOptList))
	{
		return 0;	//����	
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
