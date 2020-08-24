

#include "stdafx.h"
#include "common.h"


#define MAX_BLOCK_SIZE 4096

struct ack
{
	WORD opcode;
	union
	{
		WORD block;
		char buffer[510];
	};
};

struct packet
{
	WORD opcode;
	WORD block;
	char buffer[MAX_BLOCK_SIZE];
};

struct tftperror
{
	WORD opcode;
	WORD errorcode;
	char errormessage[508];
};

struct request
{
	char path[256];
	char errortxt[256];
	FILE *file;
	char *filename;
	char *mode;
	DWORD threadId;
	DWORD tsize;
	DWORD blksize;
	DWORD interval;
	time_t timeout;
	WORD block;
	int bytesRecd;
	int bytesRead[2];
	int bytesSent;
	SOCKADDR_IN client;
	SOCKADDR_IN sender;
	int clientsize;
	packet pkt[2];
	timeval tv;
	fd_set readfds;
	tftperror er;
	ack acout;
	union
	{
		ack acin;
		WORD opcode;
		char buffer[512];
	};
};


static BOOL verbatim = false;
static char homeDir[512];
static WORD blksize = MAX_BLOCK_SIZE;
static WORD interval = 10;
static WORD timeout = 20;


BOOL strcasecmp(char * p, char * p1)
{
	if(_stricmp(p, p1) == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


DWORD WINAPI ProcessRequest(LPVOID lpParam)
{
	SOCKET m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == INVALID_SOCKET)
	{
		if (verbatim)
		{
			LOGERROR(GetLastError(), TEXT("ProcessRequest::socket Error"));
		}
		return 1;
	}

	// Bind the socket.
	sockaddr_in service;
	service.sin_family      = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port        = 0;

	if (bind(m_socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
	{
		if (verbatim)
		{
			LOGERROR(GetLastError(), TEXT("ProcessRequest::bind Error"));	
		}
		closesocket(m_socket);
		return 1;
	}

	request *req  = (request*)lpParam;
	req->blksize  = 512;
	req->interval = interval;
	req->timeout  = timeout;
	BOOL fetchAck = false;

	char *temp = req->buffer;
	temp += 2;
	req->filename = temp;
	temp += strlen(temp) + 1;
	req->mode = temp;
	temp += strlen(temp) + 1;
	strcpy(req->path, homeDir);
	strcat(req->path, req->filename);

	for (int i = 0; i < strlen(req->path); i++)
		if (req->path[i] == '/')
			req->path[i] = '\\';

	if (strstr(req->path, "..\\"))
	{
		req->er.opcode = htons(5);
		req->er.errorcode = htons(2);
		strcpy(req->er.errormessage, "Access violation");
		req->bytesSent = sendto(m_socket, (const char*) & req->er, strlen(req->er.errormessage) + 5, 0, (SOCKADDR*) & req->client, req->clientsize);

		if (verbatim)
			printf("Client %s:%u %s, %s\n", inet_ntoa(req->client.sin_addr), htons(req->client.sin_port), req->path, req->er.errormessage);

		free(req);
		return 1;
	}

	if (htons(req->opcode) == 1)
	{
		req->file = fopen(req->path, "rb")
		            ;
		if (!req->file)
		{
			req->er.opcode = htons(5);
			req->er.errorcode = htons(1);
			strcpy(req->er.errormessage, "File Not Found");
			req->bytesSent = sendto(m_socket, (const char*) & req->er, strlen(req->er.errormessage) + 5, 0, (SOCKADDR*) & req->client, req->clientsize);

			if (verbatim)
				printf("Client %s:%u %s, %s\n", inet_ntoa(req->client.sin_addr), htons(req->client.sin_port), req->path, req->er.errormessage);

			free(req);
			return 1;
		}
	}
	else
	{
		req->file = fopen(req->path, "rb");
		if (req->file)
		{
			req->er.opcode = htons(5);
			req->er.errorcode = htons(6);
			strcpy(req->er.errormessage, "File already exists");
			req->bytesSent = sendto(m_socket, (const char*) & req->er, strlen(req->er.errormessage) + 5, 0, (SOCKADDR*) & req->client, req->clientsize);
			fclose(req->file);

			if (verbatim)
				printf("Client %s:%u %s, %s\n", inet_ntoa(req->client.sin_addr), htons(req->client.sin_port), req->path, req->er.errormessage);

			free(req);
			return 1;
		}
		else
		{
			req->file = fopen(req->path, "wb");
			if (!req->file)
			{
				req->er.opcode = htons(5);
				req->er.errorcode = htons(2);
				strcpy(req->er.errormessage, "Invalid Path");
				req->bytesSent = sendto(m_socket, (const char*) & req->er, strlen(req->er.errormessage) + 5, 0, (SOCKADDR*) & req->client, req->clientsize);

				if (verbatim)
					printf("Client %s:%u %s, %s\n", inet_ntoa(req->client.sin_addr), htons(req->client.sin_port), req->path, req->er.errormessage);

				free(req);
				return 1;
			}
		}
	}

	if (*temp)
	{
		fetchAck = true;
		char *pointer = req->acout.buffer;
		req->acout.opcode = htons(6);
		DWORD val;
		while (*temp)
		{
			//printf("%s", temp);
			if (strcasecmp(temp, "blksize"))
			{
				strcpy(pointer, temp);
				pointer += strlen(pointer) + 1;
				temp += strlen(temp) + 1;
				val = atol(temp);

				if (val > 512 && val <= blksize)
					req->blksize = val;

				sprintf(pointer, "%u", req->blksize);
				pointer += strlen(pointer) + 1;
				temp += strlen(temp) + 1;

			}
			else if (strcasecmp(temp, "tsize"))
			{
				strcpy(pointer, temp);
				pointer += strlen(pointer) + 1;
				temp += strlen(temp) + 1;
				fseek(req->file, 0, SEEK_END);
				req->tsize = ftell(req->file);
				sprintf(pointer, "%u", req->tsize);
				pointer += strlen(pointer) + 1;
				temp += strlen(temp) + 1;
			}
			else if (strcasecmp(temp, "interval"))
			{
				strcpy(pointer, temp);
				pointer += strlen(pointer) + 1;
				temp += strlen(temp) + 1;
				val = atoi(temp);

				if (val > 0 && val <= 100)
					req->interval = val;

				sprintf(pointer, "%u", req->interval);
				pointer += strlen(pointer) + 1;
				temp += strlen(temp) + 1;
			}
			else if (strcasecmp(temp, "timeout"))
			{
				strcpy(pointer, temp);
				pointer += strlen(pointer) + 1;
				temp += strlen(temp) + 1;
				val = atol(temp);

				if (val > 0 && val <= 1800)
					req->timeout = val;

				sprintf(pointer, "%u", req->timeout);
				pointer += strlen(pointer) + 1;
				temp += strlen(temp) + 1;
			}
			else
				temp += strlen(temp) + 1;

			//printf("=%u\n",val);
		}
		req->bytesSent = sendto(m_socket, (const char*) & req->acout, (DWORD)pointer - (DWORD) & req->acout, 0, (SOCKADDR*) & req->client, req->clientsize);
	}
	else if (htons(req->opcode) == 2)
	{
		req->acout.opcode = htons(4);
		req->acout.block = htons(0);
		req->bytesSent = sendto(m_socket, (const char*)&req->acout, 4, 0, (SOCKADDR*) &req->client, req->clientsize);
	}

	if (req->timeout <= req->interval)
		req->timeout = req->interval + 1;

	req->timeout += time(NULL);

	if (WSAGetLastError())
	{
		req->er.opcode = htons(5);
		req->er.errorcode = htons(0);
		sprintf(req->er.errormessage, "Winsock Error %i", WSAGetLastError());
		req->bytesSent = sendto(m_socket, (const char*) & req->er, strlen(req->er.errormessage) + 5, 0, (sockaddr*) & req->client, req->clientsize);

		if (verbatim)
			printf("Client %s:%u %s, error: %s\n", inet_ntoa(req->client.sin_addr), htons(req->client.sin_port), req->path, req->er.errormessage);

		if (req->file)
			fclose(req->file);

		free(req);
		return 1;
	}
	else if (ntohs(req->opcode) == 1)
	{
		if (ftell(req->file))
			fseek(req->file, 0, SEEK_SET);

		req->pkt[0].opcode = htons(3);
		req->pkt[0].block = htons(1);
		req->bytesRead[0] = fread(req->pkt[0].buffer, 1, req->blksize, req->file);

		if (req->bytesRead[0] == req->blksize)
		{
			req->pkt[1].opcode = htons(3);
			req->pkt[1].block = htons(2);
			req->bytesRead[1] = fread(req->pkt[1].buffer, 1, req->blksize, req->file);
			if (req->bytesRead[1] < req->blksize)
			{
				fclose(req->file);
				req->file = 0;
			}
		}
		else
		{
			fclose(req->file);
			req->file = 0;
		}

		while (true)
		{
			if (req->timeout < time(NULL))
			{
				req->er.opcode = htons(5);
				req->er.errorcode = htons(0);
				strcpy(req->er.errormessage, "Timeout");
				break;
			}

			if (fetchAck)
			{
				req->tv.tv_sec = req->interval;
				req->tv.tv_usec = 0;
				FD_ZERO(&req->readfds);
				FD_SET(m_socket, &req->readfds);
				select(USHRT_MAX, &req->readfds, NULL, NULL, &req->tv);
				if (FD_ISSET(m_socket, &req->readfds))
				{
					req->bytesRecd = recvfrom(m_socket, (char*) & req->buffer, sizeof(req->buffer), 0, (sockaddr*) & req->sender, &req->clientsize);
					if (WSAGetLastError())
						break;
					if (req->sender.sin_addr.s_addr != req->client.sin_addr.s_addr || req->sender.sin_port != req->client.sin_port)
					{
						req->er.opcode = htons(5);
						req->er.errorcode = htons(5);
						strcpy(req->er.errormessage, "Unknown Transfer ID");
						req->bytesSent = sendto(m_socket, (const char*) & req->er, strlen(req->er.errormessage) + 5, 0, (sockaddr*) & req->sender, req->clientsize);
						continue;
					}
				}
				else
					req->bytesRecd = 0;
			}
			else
				req->acin.block = htons(0);

			fetchAck = true;

			//printf("%i\n",ntohs(req->acin.opcode));

			if (req->bytesRecd >= 4)
			{
				if (ntohs(req->opcode) == 1 || ntohs(req->opcode) == 4)
				{
					if (ntohs(req->acin.block) == req->block + 1)
						req->block = ntohs(req->acin.block);

					if (req->block == USHRT_MAX)
					{
						req->er.opcode = htons(5);
						req->er.errorcode = htons(3);
						strcpy(req->er.errormessage, "File Too Large");
						break;
					}
				}
				else if (ntohs(req->opcode) == 5)
				{
					memcpy(&req->er, &req->acin, req->bytesRecd);
					break;
				}
				else
				{
					req->er.opcode = htons(5);
					req->er.errorcode = htons(0);
					strcpy(req->er.errormessage, "Unexpected Option Code");
					break;
				}
			}

			if (ntohs(req->pkt[0].block) == req->block + 1)
			{
				req->bytesSent = sendto(m_socket, (const char*) & req->pkt[0], req->bytesRead[0] + 4, 0, (sockaddr*) & req->client, req->clientsize);
				if (WSAGetLastError())
					break;
				if (req->file)
				{
					if (ntohs(req->pkt[1].block) < ntohs(req->pkt[0].block))
					{
						req->pkt[1].block = htons(req->block + 2);
						req->bytesRead[1] = fread(req->pkt[1].buffer, 1, req->blksize, req->file);
						if (req->bytesRead[1] < req->blksize)
						{
							fclose(req->file);
							req->file = 0;
						}
					}
				}
			}
			else if (ntohs(req->pkt[1].block) == req->block + 1)
			{
				req->bytesSent = sendto(m_socket, (const char*) & req->pkt[1], req->bytesRead[1] + 4, 0, (sockaddr*) & req->client, req->clientsize);
				if (WSAGetLastError())
					break;
				if (req->file)
				{
					if (ntohs(req->pkt[0].block) < ntohs(req->pkt[1].block))
					{
						req->pkt[0].block = htons(req->block + 2);
						req->bytesRead[0] = fread(req->pkt[0].buffer, 1, req->blksize, req->file);
						if (req->bytesRead[0] < req->blksize)
						{
							fclose(req->file);
							req->file = 0;
						}
					}
				}
			}
			else if (ntohs(req->pkt[0].block) == req->block || ntohs(req->pkt[1].block) == req->block)
				break;
		}

		if (verbatim && !WSAGetLastError() && !req->er.errormessage[0])
			printf("Client %s:%u %s, %u Blocks Served\n", inet_ntoa(req->client.sin_addr), ntohs(req->client.sin_port), req->path, req->block);
	}
	else if (ntohs(req->opcode) == 2)
	{
		//BYTE reSends = 0;
		while (true)
		{
			if (req->timeout < time(NULL))
			{
				req->er.opcode = htons(5);
				req->er.errorcode = htons(0);
				strcpy(req->er.errormessage, "Timeout");
				break;
			}

			req->tv.tv_sec = req->interval;
			req->tv.tv_usec = 0;
			FD_ZERO(&req->readfds);
			FD_SET(m_socket, &req->readfds);
			select(USHRT_MAX, &req->readfds, NULL, NULL, &req->tv);

			if (FD_ISSET(m_socket, &req->readfds))
			{
				req->bytesRecd = recvfrom(m_socket, (char*) & req->pkt[0], sizeof(packet), 0, (sockaddr*) & req->sender, &req->clientsize);
				if (WSAGetLastError())
					break;
				if (req->sender.sin_addr.s_addr != req->client.sin_addr.s_addr || req->sender.sin_port != req->client.sin_port)
				{
					req->er.opcode = htons(5);
					req->er.errorcode = htons(5);
					strcpy(req->er.errormessage, "Unknown Transfer ID");
					sendto(m_socket, (const char*) & req->er, strlen(req->er.errormessage) + 5, 0, (sockaddr*) & req->sender, req->clientsize);
					continue;
				}
			}
			else
				req->bytesRecd = 0;

			if (req->bytesRecd >= 4)
			{
				if (ntohs(req->pkt[0].opcode) == 3)
				{
					if (ntohs(req->pkt[0].block) == USHRT_MAX)
					{
						req->er.opcode = htons(5);
						req->er.errorcode = htons(3);
						strcpy(req->er.errormessage, "File Too Large");
						break;
					}

					req->acout.opcode = htons(4);
					if (ntohs(req->pkt[0].block) == req->block + 1)
						req->acout.block = req->pkt[0].block;
					else
						req->acout.block = htons(req->block);

					sendto(m_socket, (const char*)&req->acout, 4, 0, (sockaddr*)&req->client, req->clientsize);
					if (WSAGetLastError())
						break;

					if (!req->file)
						break;

					if (ntohs(req->pkt[0].block) == req->block + 1)
					{
						if (req->bytesRecd > 4 && !fwrite(req->pkt[0].buffer, req->bytesRecd - 4, 1, req->file))
						{
							req->er.opcode = htons(5);
							req->er.errorcode = htons(3);
							strcpy(req->er.errormessage, "Disk full or allocation exceeded");
							break;
						}

						if (req->bytesRecd - 4 == req->blksize)
						{
							req->block++;
							//reSends = 0;
						}
						else
						{
							fclose(req->file);
							req->file = 0;
							break;
						}
					}
				}
				else if (ntohs(req->pkt[0].opcode) == 5)
				{
					memcpy(&req->er, &req->pkt[0], req->bytesRecd);
					break;
				}
				else
				{
					req->er.opcode = htons(5);
					req->er.errorcode = htons(0);
					strcpy(req->er.errormessage, "Unexpected Option Code");
					break;
				}
			}
			else if (req->file)
			{
				sendto(m_socket, (const char*)&req->acout, req->bytesSent, 0, (sockaddr*)&req->client, req->clientsize);
				if (WSAGetLastError())
					break;
			}
			else
				break;
		}

		if (verbatim && !WSAGetLastError() && !req->er.errormessage[0])
			printf("Client %s:%u %s, %u Blocks Received\n", inet_ntoa(req->client.sin_addr), ntohs(req->client.sin_port), req->path, req->block + 1);
	}

	if (WSAGetLastError() || req->er.errormessage[0])
	{
		if (WSAGetLastError())
		{
			req->er.opcode = htons(5);
			req->er.errorcode = htons(0);
			sprintf(req->er.errormessage, "WinSock Error:%i", WSAGetLastError());
		}
		if (verbatim)
			printf("Client %s:%u %s, error: %s\n", inet_ntoa(req->client.sin_addr), htons(req->client.sin_port), req->path, req->er.errormessage);
		req->bytesSent = sendto(m_socket, (const char*) & req->er, strlen(req->er.errormessage) + 5, 0, (sockaddr*) & req->client, req->clientsize);
	}

	if (req->file)
		fclose(req->file);

	closesocket(m_socket);
	free(req);
	return 0;
}

extern CHAR g_szTempPath[MAX_PATH];

extern BOOL g_bIsTerminateThread;

ULONG TFTPServer(LPVOID lParam)
{
	PNETCARD_INFO lpNetInfo = (PNETCARD_INFO)lParam;
	lstrcpy(homeDir, g_szTempPath);
	char * x = strrchr(homeDir, '\\') + 1;
	*x = 0;
	verbatim = true;
	// Create a socket.
	SOCKET m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == INVALID_SOCKET)
	{
		LOGERROR(GetLastError(), TEXT("TFTPServer::socket Error"));
		return 0;
	}
	
	// Bind the socket.
	sockaddr_in service;
	service.sin_family      = AF_INET;
	service.sin_addr.s_addr = inet_addr(lpNetInfo->strIP);
	service.sin_port        = htons(69);
	
	BOOL optval = TRUE;
	//SO_REUSEADDR选项就是可以实现端口重绑定的
	if(setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (PCHAR)&optval, sizeof(optval)) == SOCKET_ERROR)
	{
		LOGERROR(GetLastError(), TEXT("TFTPServer::SO_REUSEADDR Error"));
		return GetLastError();
	}

	if (bind(m_socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
	{
		LOGERROR(GetLastError(), TEXT("TFTPServer::bind Error"));
		closesocket(m_socket);
		return 0;
	}

//	printf("Home Directory: %s\n", homeDir);
//	printf("max blksize: %u\n", blksize);
//	printf("defult blksize: %u\n", 512);
//	printf("default interval: %u\n", interval);
//	printf("defalut timeout: %u\n", timeout);
//	printf( "\nReady...\n" );
	
	timeval tv;
	fd_set readfds;
	
	do
	{
		tv.tv_sec = 10;
		tv.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_SET(m_socket, &readfds);
		select(USHRT_MAX, &readfds, NULL, NULL, &tv);
		if(g_bIsTerminateThread)
		{
			LOGERROR(0, TEXT("TFTPServer Is Exit"));
			break;
		}
		if (FD_ISSET(m_socket, &readfds))
		{
			request * req = (request*)malloc(sizeof(request));
			memset(req, 0, sizeof(request));
			req->clientsize = sizeof(req->client);
			req->bytesRecd = recvfrom(m_socket, req->buffer, sizeof(req->buffer), 0, (SOCKADDR*) & req->client, &req->clientsize);
			if (!WSAGetLastError() && req->bytesRecd > 0 && (htons(req->opcode) == 1 || htons(req->opcode) == 2))
			{
				HANDLE hThread = CreateThread(
					NULL,     				// default security attributes
					0,     				// use default stack size
					ProcessRequest,     	// thread function
					req,     				// argument to thread function
					0,     				// use default creation flags
					&req->threadId);	// returns the thread identifier
				
				// Check the return value for success.
				
				if (hThread == NULL)
				{
					LOGERROR(GetLastError(), TEXT("TFTPServer::Create Thread Failed.."));
				}
			}
			else
			{
				free(req);
			}
		}
	}
	while (TRUE);
	closesocket(m_socket);
	return 0;
}


void ShellTFTPServerWrap(LPVOID lParam)
{
	TFTPServer(lParam);
}