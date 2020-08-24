
#include "stdafx.h"
#include "common.h"

#define DHCP_PORT			4011


struct pxe_boot_list
{
	int len;
	int size;
	char *data;
};


void PXE_BootListInit(struct pxe_boot_list *list, char *data, int data_size)
{
	list->len  = 0;
	list->size = data_size;
	list->data = data;
}


int PXE_BootListAdd(struct pxe_boot_list *list, WORD type, BYTE ip_cnt, DWORD *ips)
{
	if((list->len + 3 + 4 * ip_cnt) > list->size) 
	{
		return -1;
	}
	
	memcpy(list->data + list->len, &type, sizeof(type));
	list->data[list->len + 2] = ip_cnt;
	memcpy(list->data + list->len + 3, ips, ip_cnt * 4);
	
	list->len += 3 + 4 * ip_cnt;
	
	return 0;
}


int PXE_BootListLen(struct pxe_boot_list *list)
{
	return list->len;
}


struct pxe_menu 
{
	int len;
	int size;
	char *data;
};


void PXE_MenuInit(struct pxe_menu *menu, char *data, int data_size)
{
	menu->len = 0;
	menu->size = data_size;
	menu->data = data;
}


int PXE_MenuAdd(struct pxe_menu *menu, WORD item_num, char *name)
{
	int len = strlen((char*)name);
	if((menu->len + len + 3) > menu->size)
	{
		return -1;
	}
	
	memcpy(menu->data + menu->len, &item_num, sizeof(item_num));
	menu->data[menu->len + 2] = len;
	memcpy(menu->data + menu->len + 3, name, len);
	
	menu->len += len + 3;
	
	return 0;
}


int PXE_MenuLen(struct pxe_menu *menu)
{
	return menu->len;
}


void PXE_MakeFast(struct dhcp_packet * pPacket, struct dhcp_packet * pReply, ULONG uSelfIP)
{
	struct dhcp_packet & stReply = *pReply;
	static CHAR          szBootFile[128];
	ZeroMemory(szBootFile, sizeof(szBootFile));
	memset(&stReply, 0, sizeof(stReply));
	stReply.op     = DHCP_OP_BOOTREPLY;
	stReply.htype  = pPacket->htype;
	stReply.hlen   = pPacket->hlen;
	stReply.hops   = pPacket->hops;
	stReply.xid    = pPacket->xid;
	stReply.secs   = 4;
	stReply.flags  = pPacket->flags;
	stReply.ciaddr = pPacket->ciaddr;
	stReply.yiaddr = pPacket->yiaddr;
	stReply.siaddr = htonl(uSelfIP); //此字段指出TFTP服务器所在IP地址
	stReply.giaddr = pPacket->giaddr;
	memcpy(stReply.chaddr, pPacket->chaddr, 16);
	stReply.sname  = NULL;
	stReply.file   = szBootFile; //引导文件名
	lstrcpy(szBootFile, g_szBootFileName);

	PBYTE pbMac = (PBYTE)stReply.chaddr;
	CHAR strMac[48];
	wsprintf(strMac,"%02x%02x%02x%02x%02x%02x",pbMac[0],pbMac[1],pbMac[2],pbMac[3],pbMac[4],pbMac[5]);
	_strupr(strMac);
	
	//一定要加static
	static BYTE DHCP_MsgType;
	DHCP_MsgType = DHCP_ACK;
	DHCP_SetOption(&stReply, DHCP_OPTION_MSG_TYPE, 1, &DHCP_MsgType);
	
	//套接字绑定的IP,一定要加static	//自己的IP
	static DWORD ServerIP;
	ServerIP = uSelfIP;	//这里是小尾方式
	
	DHCP_SetOption(&stReply, DHCP_OPTION_SERVER_ID, 4, &ServerIP);
	
	static CHAR uuid[17];	//一定要加static
	if(pPacket->options[DHCP_OPTION_CLIENT_UUID].len != 0)
	{
		memcpy(uuid, pPacket->options[DHCP_OPTION_CLIENT_UUID].data, 17);
		DHCP_SetOption(&stReply, DHCP_OPTION_CLIENT_UUID, 17, uuid);
	}
	
	//这里可以不要加static,因为字符串常量是放在const段里面
	PCHAR class_id = "PXEClient";
	DHCP_SetOption(&stReply, DHCP_OPTION_CLASS_ID, strlen(class_id), class_id);
}


CRITICAL_SECTION g_csPxeLock;

/*PXE包的处理
参数一:接收到的数据包的首地址
参数二:接收到的数据包的长度
参数三:存放应答数据的缓冲区首地址
参数四:缓冲区长度
参数五:本线程绑定的IP
*/
ULONG PXE_Handle(PCHAR pData, ULONG nDataLen, PCHAR pSendBuf, ULONG nBufLen, ULONG uSelfIP)
{
	struct dhcp_packet stPacket;	//存放接收数据包的结构
	struct dhcp_packet stReply;		//存放应答数据包的结构

	if (!DHCP_ParsePacket(pData, nDataLen, &stPacket))
	{
		return 0;	
	}

	//只处理DHCP_REQUEST包
	BYTE nMsgType = stPacket.options[DHCP_OPTION_MSG_TYPE].data[0];
	if (nMsgType != DHCP_REQUEST)
	{
		return 0;
	}

	ULONG uLenght = 0;

	EnterCriticalSection(&g_csPxeLock);

	switch(stPacket.options[DHCP_OPTION_MSG_TYPE].data[0])
	{
	case DHCP_REQUEST:
		PXE_MakeFast(&stPacket, &stReply, uSelfIP);
		uLenght = DHCP_MakeSendData(&stReply, pSendBuf, nBufLen);
		break;
	}

	LeaveCriticalSection(&g_csPxeLock);

	return uLenght;
}


extern BOOL g_bIsTerminateThread;	//外部变量


DWORD WINAPI PXE_Service(LPVOID lParam)
{
	struct sockaddr_in stServAddr;
	PNETCARD_INFO      lpNetInfo = (PNETCARD_INFO)lParam;

	SOCKET hSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (hSocket == INVALID_SOCKET)
	{
		LOGERROR(GetLastError(), TEXT("PXE_Service::socket Error"));
		return GetLastError();
	}

	memset(&stServAddr, 0, sizeof(stServAddr));
	
	stServAddr.sin_family      = AF_INET;
	stServAddr.sin_addr.s_addr = inet_addr(lpNetInfo->strIP);
	stServAddr.sin_port        = htons(DHCP_PORT);

	BOOL optval = TRUE;
	//SO_REUSEADDR选项就是可以实现端口重绑定的
	if(setsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR, (PCHAR)&optval, sizeof(optval)) == SOCKET_ERROR)
	{
		LOGERROR(GetLastError(), TEXT("PXE_Service::SO_REUSEADDR Error"));
		return GetLastError();	
	}

	int nRet = bind(hSocket, (struct sockaddr *)&stServAddr, sizeof(stServAddr));
	if (nRet == SOCKET_ERROR) 
	{
		LOGERROR(GetLastError(), TEXT("PXE_Service::bind Error"));
		return GetLastError();
	}

	struct sockaddr_in stClientAddr;
	int                nClientLen;
	int                nRecvLen;
	CHAR               szBuf[MAX_PACKET_LEN];		//接收的数据存放处
	CHAR               szSendBuf[MAX_PACKET_LEN];	//准备发送的数据存放处
	
	while(TRUE) 
	{
		nClientLen = sizeof(stClientAddr);
		nRecvLen = recvfrom(hSocket, szBuf, MAX_PACKET_LEN, 0, (struct sockaddr *)&stClientAddr, &nClientLen);

		if(g_bIsTerminateThread)
		{
			LOGERROR(0, TEXT("PXE_Service Is Exit"));
			break;
		}

		if(nRecvLen == 0)
		{
			continue;		
		}
		if (nRecvLen == SOCKET_ERROR)
		{
			//break;
			LOGERROR(GetLastError(), TEXT("PXE_Service::recvfrom Error"));
			continue;
		}

		//只有在客户端有IP的情况下才处理
		if(stClientAddr.sin_addr.S_un.S_addr == 0)
		{
			continue;		
		}

		struct sockaddr_in stSelfAddr;
		int                len = sizeof(stSelfAddr) ;
		memset(&stSelfAddr, 0, sizeof(stSelfAddr));
		getsockname(hSocket, (struct sockaddr *)&stSelfAddr, &len);//取套接字绑定的IP

		int nSendLen = PXE_Handle(szBuf, nRecvLen, szSendBuf, sizeof(szSendBuf), stServAddr.sin_addr.S_un.S_addr);
		if (nSendLen == 0)
		{
			//处理失败
			LOGERROR(GetLastError(), TEXT("PXE_Handle nSendLen==0"));
			continue;
		}
		else	//成功~发送数据
		{
			int nLen = sendto(hSocket, szSendBuf, nSendLen, 0, (struct sockaddr *)&stClientAddr, sizeof(stClientAddr));
			if (nLen == SOCKET_ERROR) 
			{
				LOGERROR(GetLastError(), TEXT("PXE_Service::sendto Error"));				
			}
		}
	}
	closesocket(hSocket);
	return 0;
}


void ShellPXEServiceWrap(LPVOID lParam)
{
	PXE_Service(lParam);
}
