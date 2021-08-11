#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include<stdio.h>
#include<thread>
#include<vector>

#pragma comment(lib,"ws2_32.lib")

std::vector<SOCKET> g_clients;

int processor(SOCKET _cSock)
{
	//������
	char szRecv[4096] = {};
	// 5 ���տͻ�������
	int nLen = (int)recv(_cSock, szRecv, sizeof(szRecv), 0);	
	if (nLen <= 0)
	{
		printf("�ͻ���<Socket=%d>���˳������������\n", _cSock);
		return -1;
	}
	for (int n = (int)g_clients.size() - 1; n >= 0; n--)
	{
		send(g_clients[n], (char*)&szRecv, nLen, 0);
	}
	return 0;
}

int main()
{
#ifdef _WIN32
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//------------
#endif
	//-- ��Socket API��������TCP�����
	// 1 ����һ��socket �׽���
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// 2 bind �����ڽ��ܿͻ������ӵ�����˿�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//host to net unsigned short
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
#else
	_sin.sin_addr.s_addr = INADDR_ANY;
#endif
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("����,������˿�ʧ��...\n");
	}
	else {
		printf("������˿ڳɹ�...\n");
	}
	// 3 listen ��������˿�
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("����,��������˿�ʧ��...\n");
	}
	else {
		printf("��������˿ڳɹ�...\n");
	}

	while (true)
	{
		//�������׽��� BSD socket
		fd_set fdRead;//��������socket�� ����
		fd_set fdWrite;
		fd_set fdExp;
		//������
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);
		//����������socket�����뼯��
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);
		SOCKET maxSock = _sock;
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
			if (maxSock < g_clients[n])
			{
				maxSock = g_clients[n];
			}
		}
		
		timeval t = { 1,0 };
		int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret < 0)
		{
			printf("select���������\n");
			break;
		}
		//�ж���������socket���Ƿ��ڼ�����
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			// 4 accept �ȴ����ܿͻ�������
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
			_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
			if (INVALID_SOCKET == _cSock)
			{
				printf("����,���ܵ���Ч�ͻ���SOCKET...\n");
			}
			else
			{
				/*const char* join_msg = "New Clients join in.\n";
				for (int n = (int)g_clients.size() - 1; n >= 0; n--)
				{					
					send(g_clients[n], (const char*)join_msg, sizeof(join_msg), 0);
				}
				*/
				g_clients.push_back(_cSock);
				printf("�¿ͻ��˼��룺socket = %d,IP = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
			}
		}
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			if (FD_ISSET(g_clients[n], &fdRead))
			{
				if (-1 == processor(g_clients[n]))
				{
#ifdef _WIN32
					closesocket(g_clients[n]);
#else
					close(g_clients[n]);
#endif
					auto iter = g_clients.begin() + n;//std::vector<SOCKET>::iterator
					if (iter != g_clients.end())
					{
						g_clients.erase(iter);
					}

				}
			}
		}
		//printf("����ʱ�䴦������ҵ��..\n");
	}
#ifdef _WIN32
	for (int n = (int)g_clients.size() - 1; n >= 0; n--)
	{
		closesocket(g_clients[n]);
	}
	// 8 �ر��׽���closesocket
	closesocket(_sock);
	//------------
	//���Windows socket����
	WSACleanup();
#else
	for (int n = (int)g_clients.size() - 1; n >= 0; n--)
	{
		close(g_clients[n]);
	}
	// 8 �ر��׽���closesocket
	close(_sock);
#endif
	printf("���˳���\n");
	getchar();
	return 0;
}