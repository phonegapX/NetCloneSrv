
#include "stdafx.h"
#include "common.h"

#define DHCP_PORT			67

//构造ACK类型的包
//根据接收的数据包的dhcp_packet结构进行一系列处理后
//构造出 待发送的数据包的dhcp_packet结构
//参数一:接收的数据包的dhcp_packet结构(输入参数)
//参数二:待发送的数据包的dhcp_packet结构(输出参数)
//参数三:本线程绑定的IP
//参数四:发请求的客户端的网卡MAC地址
//参数五:WKS.ini的对应的类
void DHCP_MakeAck(struct dhcp_packet * pPacket, struct dhcp_packet * pReply, ULONG uSelfIP, PCHAR strMac, PNETCARD_INFO lpNetInfo)
{
	struct dhcp_packet & stReply = *pReply;
	
	memset(&stReply, 0, sizeof(stReply));
	
	stReply.op    = DHCP_OP_BOOTREPLY;
	stReply.htype = pPacket->htype;
	stReply.hlen  = pPacket->hlen;
	stReply.hops  = pPacket->hops;
	stReply.xid   = pPacket->xid;
	stReply.secs  = 4;
	stReply.flags = pPacket->flags;
	stReply.ciaddr= pPacket->ciaddr;
	//分配给客户端的IP,从WKS.ini中读

	CHAR szIP[48] = {0};
	IPPool_GetIPAddress(strMac, szIP);
	ULONG uCliIP = inet_addr(szIP);
	stReply.yiaddr = htonl(uCliIP);
	//套界字绑定的IP
	stReply.siaddr = htonl(uSelfIP);
	stReply.giaddr = pPacket->giaddr;
	memcpy(stReply.chaddr, pPacket->chaddr, 16);
	stReply.sname = NULL;
	stReply.file = NULL;

	//一定要加static
	static BYTE DHCP_MsgType;
	DHCP_MsgType = DHCP_ACK;
	DHCP_SetOption(&stReply, DHCP_OPTION_MSG_TYPE, 1, &DHCP_MsgType);
	
	//套界字绑定的IP,一定要加static
	static DWORD ServerIP;
	ServerIP = uSelfIP;
	DHCP_SetOption(&stReply, DHCP_OPTION_SERVER_ID, 4, &ServerIP);
	
	static CHAR uuid[17];	//一定要加static
	if(pPacket->options[DHCP_OPTION_CLIENT_UUID].len != 0)
	{
		memcpy(uuid, pPacket->options[DHCP_OPTION_CLIENT_UUID].data, 17);
		DHCP_SetOption(&stReply, DHCP_OPTION_CLIENT_UUID, 17, uuid);
	}

	static DWORD mask_id;
	mask_id = inet_addr(lpNetInfo->strNetmask);
	DHCP_SetOption(&stReply, DHCP_OPTION_MASK, 4, &mask_id);
	
	//这里可以不要加static,因为字符串常量是放在const段里面
	PCHAR class_id = "PXEClient";
	DHCP_SetOption(&stReply, DHCP_OPTION_CLASS_ID, strlen(class_id), class_id);

//===================================================================
// DHCP协议需要用到
	static DWORD LeaseTime;
	LeaseTime = htonl(172800);
	DHCP_SetOption(&stReply, DHCP_OPTION_LEASETIME, 4, &LeaseTime);
	
	static DWORD Renewal;
	Renewal = htonl(86400);
	DHCP_SetOption(&stReply, DHCP_OPTION_RENEWAL, 4, &Renewal);
	
	
	static DWORD Rebinding;
	Rebinding = htonl(86400);
	DHCP_SetOption(&stReply, DHCP_OPTION_REBINDING, 4, &Rebinding);
}

//构造Offer类型的包
//根据接收的数据包的dhcp_packet结构进行一系列处理后
//构造出 待发送的数据包的dhcp_packet结构
//参数一:接收的数据包的dhcp_packet结构(输入参数)
//参数二:待发送的数据包的dhcp_packet结构(输出参数)
//参数三:本线程绑定的IP
//参数四:发请求的客户端的网卡MAC地址
//参数五:WKS.ini的对应的类
void DHCP_MakeOffer(struct dhcp_packet * pPacket, struct dhcp_packet * pReply, ULONG uSelfIP, PCHAR strMac, PNETCARD_INFO lpNetInfo)
{
	struct dhcp_packet & stReply = *pReply;

	memset(&stReply, 0, sizeof(stReply));

	stReply.op    = DHCP_OP_BOOTREPLY;
	stReply.htype = pPacket->htype;
	stReply.hlen  = pPacket->hlen;
	stReply.hops  = pPacket->hops;
	stReply.xid   = pPacket->xid;
	stReply.secs  = 4;
	stReply.flags = pPacket->flags;
	stReply.ciaddr= pPacket->ciaddr;
	//分配给客户端的IP,从WKS.ini中读

	CHAR szIP[48] = {0};
	IPPool_GetIPAddress(strMac, szIP);
	ULONG uCliIP = inet_addr(szIP);
	stReply.yiaddr = htonl(uCliIP);
	//套接字绑定的IP
	stReply.siaddr = htonl(uSelfIP);
	stReply.giaddr = pPacket->giaddr;
	memcpy(stReply.chaddr, pPacket->chaddr, 16);
	stReply.sname = NULL;
	stReply.file = NULL;

	//一定要加static
	static BYTE DHCP_MsgType;
	DHCP_MsgType = DHCP_OFFER;
	DHCP_SetOption(&stReply, DHCP_OPTION_MSG_TYPE, 1, &DHCP_MsgType);

	//套接字绑定的IP,一定要加static
	static DWORD ServerIP;
	ServerIP = uSelfIP;	//这里是小尾方式
	DHCP_SetOption(&stReply, DHCP_OPTION_SERVER_ID, 4, &ServerIP);

	static CHAR uuid[17];	//一定要加static
	if(pPacket->options[DHCP_OPTION_CLIENT_UUID].len != 0)
	{
		memcpy(uuid, pPacket->options[DHCP_OPTION_CLIENT_UUID].data, 17);
		DHCP_SetOption(&stReply, DHCP_OPTION_CLIENT_UUID, 17, uuid);
	}

	static DWORD mask_id;
	mask_id = inet_addr(lpNetInfo->strNetmask);
	DHCP_SetOption(&stReply, DHCP_OPTION_MASK, 4, &mask_id);

	//这里可以不要加static,因为字符串常量是放在const段里面
	PCHAR class_id = "PXEClient";
	DHCP_SetOption(&stReply, DHCP_OPTION_CLASS_ID, strlen(class_id), class_id);

//===================================================================
// DHCP协议需要用到
	static DWORD LeaseTime;
	LeaseTime = htonl(172800);
	DHCP_SetOption(&stReply, DHCP_OPTION_LEASETIME, 4, &LeaseTime);

	static DWORD Renewal;
	Renewal = htonl(86400);
	DHCP_SetOption(&stReply, DHCP_OPTION_RENEWAL, 4, &Renewal);


	static DWORD Rebinding;
	Rebinding = htonl(86400);
	DHCP_SetOption(&stReply, DHCP_OPTION_REBINDING, 4, &Rebinding);

}


CRITICAL_SECTION g_csDhcpLock;


/*DHCP包的处理
参数一:接收到的数据包的首地址
参数二:接收到的数据包的长度
参数三:存放应答数据的缓冲区首地址
参数四:缓冲区长度
参数五:本线程绑定的IP
*/
DWORD DHCP_Handle(PCHAR pData, ULONG nDataLen, PCHAR pSendBuf, ULONG nBufLen, ULONG uSelfIP, PNETCARD_INFO lpNetInfo)
{
	struct dhcp_packet stPacket;	//存放接收数据包的结构
	struct dhcp_packet stReply;		//存放应答数据包的结构

	if (!DHCP_ParsePacket(pData, nDataLen, &stPacket))
	{
		return 0;
	}

	//只处理DHCP_REQUEST包和DHCP_DISCOVER包
	BYTE nMsgType = stPacket.options[DHCP_OPTION_MSG_TYPE].data[0];
	if (!(nMsgType == DHCP_REQUEST || nMsgType == DHCP_DISCOVER))
	{
		return 0;
	}

	//当包类型是DHCP_REQUEST时候，就要判断DHCP包里面包含的SERVER_IP是不是
	//接收这个数据包的线程绑定的IP,因为这个时候DHCP是广播包,没有目的地址
	//必须有一个字段指出DHCP客户端愿意接受哪个服务端的数据
	if (stPacket.options[DHCP_OPTION_MSG_TYPE].data[0] == DHCP_REQUEST)
	{
		PULONG lpData = (PULONG)(stPacket.options[DHCP_OPTION_SERVER_ID].data);
		if (lpData == NULL) 
		{
			return 0;	//客户端没有接受我
		}
		if (*lpData != uSelfIP)
		{
			return 0;	//客户端没有接受我
		}

		//(↓这个步骤没必要做,因为这里不会出这样的情况,出了也没关系)
		//按理说还应该提取包里面分给客户端的IP地址(在前一个DHCP_DISCOVER分配的)
		//和WKS表中相应的MAC对应的
		//IP地址对比,不一样就不处理或是发NAK包
	}

	//下面开始做与实际的工作,如写数据库等

	PBYTE pbMac = (PBYTE)stPacket.chaddr;
	CHAR  strMac[48];
	wsprintf(strMac, "%02x%02x%02x%02x%02x%02x", pbMac[0], pbMac[1], pbMac[2], pbMac[3], pbMac[4], pbMac[5]);
	_strupr(strMac);

	EnterCriticalSection(&g_csDhcpLock);		//防止多网卡情况下编几个相同的号
	//如果WKS表中没有对应记录,对比MAC地址
	if (!IPPool_UserIsExist(strMac))
	{
		IPPool_AddUser(strMac);
	}
	LeaveCriticalSection(&g_csDhcpLock);

	ULONG uLength = 0;

	EnterCriticalSection(&g_csDhcpLock);

	switch(stPacket.options[DHCP_OPTION_MSG_TYPE].data[0]) 
	{
	case DHCP_DISCOVER:
		DHCP_MakeOffer(&stPacket, &stReply, uSelfIP, strMac, lpNetInfo);
		uLength = DHCP_MakeSendData(&stReply, pSendBuf, nBufLen);
		break;
			
	case DHCP_REQUEST:
		DHCP_MakeAck(&stPacket, &stReply, uSelfIP, strMac, lpNetInfo);
		uLength = DHCP_MakeSendData( &stReply, pSendBuf, nBufLen);
		break;
	}

	LeaveCriticalSection(&g_csDhcpLock);

	return uLength;
}

extern BOOL g_bIsTerminateThread;	//外部变量

DWORD WINAPI DHCP_Service(LPVOID lParam)
{
	struct sockaddr_in stServAddr;
	PNETCARD_INFO      lpNetInfo = (PNETCARD_INFO)lParam;

	SOCKET hSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (hSocket == INVALID_SOCKET) 
	{
		LOGERROR(GetLastError(), TEXT("DHCP_Service::socket Error"));
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
		LOGERROR(GetLastError(), TEXT("DHCP_Service::SO_REUSEADDR Error"));
		return GetLastError();	
	}

	int nRet = bind(hSocket, (struct sockaddr *)&stServAddr, sizeof(stServAddr));
	if (nRet == SOCKET_ERROR) 
	{
		LOGERROR(GetLastError(), TEXT("DHCP_Service::bind Error"));
		return GetLastError();
	}

	//设置套接字可以发送广播包
	optval = TRUE;
	nRet   = setsockopt(hSocket, SOL_SOCKET, SO_BROADCAST, (PCHAR)&optval, sizeof(optval));
	if(nRet == SOCKET_ERROR)
	{
		LOGERROR(GetLastError(), TEXT("DHCP_Service::setsockopt Error"));
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
		nRecvLen   = recvfrom(hSocket, szBuf, MAX_PACKET_LEN, 0, (struct sockaddr *)&stClientAddr, &nClientLen);

		if (g_bIsTerminateThread) 
		{
			LOGERROR(0, TEXT("DHCP_Service Is Exit"));
			break;
		}

		if(nRecvLen == 0)
		{
			continue;		
		}
		if (nRecvLen == SOCKET_ERROR)
		{
			LOGERROR(GetLastError(), TEXT("DHCP_Service::recvfrom Error"));
			continue;	
		}

		//判断来访的IP是不是没有,没有就设置为广播地址
		//有就证明这是一个非法的数据包,不处理
		if(stClientAddr.sin_addr.S_un.S_addr == 0)
		{
			stClientAddr.sin_addr.S_un.S_addr = INADDR_BROADCAST;	
		}
		else
		{
			continue;
		}

		struct sockaddr_in stSelfAddr;
		int                len = sizeof(stSelfAddr);
		memset(&stSelfAddr, 0, sizeof( stSelfAddr ));
		getsockname(hSocket, (struct sockaddr *)&stSelfAddr, &len);//取套接字绑定的IP

		int nSendLen = DHCP_Handle(szBuf, nRecvLen, szSendBuf, sizeof(szSendBuf), stServAddr.sin_addr.S_un.S_addr, lpNetInfo);
		if (nSendLen == 0)
		{
			//处理失败
			LOGERROR(GetLastError(), TEXT("DHCP_Handle nSendLen==0"));
			continue;
		}
		else	//成功~发送数据
		{
			int nLen = sendto(hSocket, szSendBuf, nSendLen, 0, (struct sockaddr *)&stClientAddr, sizeof(stClientAddr));
			if (nLen == SOCKET_ERROR) 
			{
				LOGERROR(GetLastError(), TEXT("DHCP_Service::sendto Error"));				
			}
		}
	}
	closesocket(hSocket);
	return ERROR_SUCCESS;
}


void ShellDHCPServiceWrap(LPVOID lParam)
{
 	DHCP_Service(lParam);
}
