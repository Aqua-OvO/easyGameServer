#ifndef EASY_TCP_SERVER_HPP
#define EASY_TCP_SERVER_HPP

#ifdef _WIN32
#define FD_SETSIZE		2600
#define WIN32_LEAN_AND_MEAN //����windows��WinSock2�ض���
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<windows.h>
#include<WinSock2.h>
#ifndef socklen_t
typedef int socklen_t;
#endif
#else
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include<algorithm>

#define SOCKET int
#define INVALID_SOCKET (SOCKET)(-0)
#define SOCKET_ERROR           (-1)

#endif
#define _CellServer_THREAD_COUNT 4

#include<stdio.h>
#include<vector>
#include<thread>
#include<mutex>
#include<atomic>
#include<iostream>

#include "CELLTimestamp.hpp"
#include "MessageHeader.hpp"

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif

class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
	}
	SOCKET sockfd()
	{
		return _sockfd;
	}
	char* msgBuf()
	{
		return _szMsgBuf;
	}
	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}
private:
	SOCKET _sockfd;
	// �ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	int _lastPos = 0;
};

class INetEvent
{
public:
	//�ͻ����뿪�¼�
	virtual void OnLeave(ClientSocket* pclient) = 0; //���麯�������м̳������Ķ���Ҫʵ���������
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header) = 0;
private:
};

class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pthread = nullptr;
		_recvCount = 0;
		_pNetEvent = nullptr;
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	~CellServer()
	{
		Close();
		_sock = INVALID_SOCKET;
	}

	//int _nCount = 0;
// ����������Ϣ
	bool OnRun()
	{
		while (isRun())
		{
			if (_clientsBuff.size() > 0)
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients.push_back(pClient);
				}
				_clientsBuff.clear();
			}
			//���û����Ҫ����Ŀͻ��ˣ�������
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//�������׽��� BSD socket
			fd_set fdRead;	//socket����
			//fd_set fdWrite;
			//fd_set fdExp;

			FD_ZERO(&fdRead);
			FD_SET(_sock, &fdRead);	//��socket���뵽socket����
			SOCKET maxSock = _clients[0]->sockfd();
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock < _clients[n]->sockfd())
				{
					maxSock = _clients[n]->sockfd();
				}
			}
			///ndfs��һ������ֵ����ָfd_set����������socket�ķ�Χ��������������
			///������socket���ֵ+1����windows��������������� 
			///��5��������NULL��������
			timeval t = { 0, 5000 };
			int ret = select((int)maxSock + 1, &fdRead, nullptr, nullptr, nullptr);
			if (ret < 0) // �����쳣
			{
				printf("select�������\n");
				Close();
				return false;
			}
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
				{
					if (-1 == RecvData(_clients[n]))
					{
						auto iter = _clients.begin() + n;
						if (iter != _clients.end())
						{
							if (_pNetEvent)
								_pNetEvent->OnLeave(_clients[n]);
							delete _clients[n];
							_clients.erase(iter);
						}
					}
				}
			}
		}
		Close();
		return false;
	}

	// �ж�����״̬
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	// �ر�socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			// 8 �ر��׽���closesocket
			closesocket(_sock);
			//----------
			//���Windows socket���绷��
			WSACleanup();// �ر�windows socket���绷��
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
				delete _clients[n];
			}
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
			_clients.clear();
		}
	}

	// ��������
	int RecvData(ClientSocket* pClient)
	{
		// ������
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		//printf("nLen=%d\n", nLen);
		//LoginResult ret;
		//SendData(_cSock, &ret);
		if (nLen < 0)
			return -1;
		// ���յ������ݿ�������Ϣ������
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		// ��Ϣ������������β��λ�ú���
		pClient->setLastPos(pClient->getLastPos() + nLen);
		// �ж���Ϣ�������ĳ����Ƿ������ϢͷDataHeader
		while (pClient->getLastPos() >= sizeof(DataHeader))
		{
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			if (pClient->getLastPos() >= header->dataLength)
			{
				// ʣ��δ������Ϣ���������ݳ���
				int nSize = pClient->getLastPos() - header->dataLength;
				// ����������Ϣ
				OnNetMsg(pClient->sockfd(), header);
				// ����Ϣ������ʣ��δ��������ǰ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				// ��Ϣ������������β��λ��ǰ��
				pClient->setLastPos(nSize);
			}
			else
			{
				// ��Ϣ���������ݲ���һ��������Ϣ
				break;
			}
		}
		return 0;
	}

	// ��Ӧ��������
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		_recvCount++;
		_pNetEvent->OnNetMsg(cSock, header);
		//double t1 = _tTime.getElapsedSecond();
		/*if (t1 >= 1.0)
		{
			printf("time<%1f>,socket<%d>,clients<%d>,_recvCount<%d>\n", t1, (int)_sock, _clients.size(), _recvCount);
			_recvCount = 0;
			_tTime.update();
		}*/
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGIN ���ݳ��ȣ�%d, userName=%s PassWord=%s\n", (int)cSock, login->dataLength, login->userName, login->passWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			LoginResult ret;
			SendData(cSock, &ret);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGOUT ���ݳ��ȣ�%d, userName=%s\n", (int)cSock, logout->dataLength, logout->userName);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			LogoutResult ret;
			SendData(cSock, &ret);
		}
		break;
		default:
			printf("<socket=%d>�յ�δ������Ϣ�����ݳ��ȣ�%d\n", (int)cSock, header->dataLength);
			// DataHeader header;
			// SendData(cSock, &header);
			break;
		}
	}

	// ����ָ��socket����
	void SendData(SOCKET _cSock, DataHeader* header)
	{
		if (isRun() && header)
			send(_cSock, (const char*)header, header->dataLength, 0);
	}

	void addClient(ClientSocket* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuff.push_back(pClient);
	}

	void Start()
	{
		_pthread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
		//_pthread->detach();
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
private:
	SOCKET _sock;
	//��ʽ�ͻ�����
	std::vector<ClientSocket*> _clients;
	//����ͻ�����
	std::vector<ClientSocket*> _clientsBuff;
	std::mutex _mutex;
	std::thread* _pthread;
	char _szRecv[RECV_BUFF_SIZE];
	INetEvent* _pNetEvent;
public:
	std::atomic_int _recvCount;
};

class EasyTcpServer : public INetEvent
{
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		//memset(_szRecv, 0, sizeof(_szRecv));
		//_recvCount = 0;
	}

	virtual ~EasyTcpServer()
	{
		Close();
	}
	// ��ʼ��socket
	void InitSocket()
	{
#ifdef _WIN32
		//����windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("�رվ�����<socket=%d>..\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("���󣬽���Socketʧ��...\n");
		}
		else
		{
			printf("<socket=%d>�����ɹ�...\n", (int)_sock);
		}
	}
	// �󶨶˿ں�
	SOCKET Bind(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(4567); // host to net unsigned short
#ifdef _WIN32    
		if (ip)
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		else
			_sin.sin_addr.S_un.S_addr = INADDR_ANY; // ������ַ��ػ���ַ���ɣ�inet_addr("127.0.0.1"); 
#else
		if (ip)
			_sin.sin_addr.s_addr = inet_addr(ip);
		else
			_sin.sin_addr.s_addr = INADDR_ANY; // ������ַ��ػ���ַ���ɣ�inet_addr("127.0.0.1"); 
#endif    
		if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
		{
			printf("<socket=%d>����,������˿�<%d>ʧ��...\n", (int)_sock, port);
		}
		else
		{
			printf("<socket=%d>������˿�<%d>�ɹ�...\n", (int)_sock, port);
		}
		return _sock;
	}
	// �����˿ں�
	int Listen(int n)
	{
		// listen ��������˿�
		int ret = listen(_sock, 5);
		if (SOCKET_ERROR == ret)
		{
			printf("<socket=%d>����,��������˿�ʧ��...\n", (int)_sock);
		}
		else
		{
			printf("<socket=%d>��������˿ڳɹ�...\n", (int)_sock);
		}
		return ret;
	}
	// ���ܿͻ�������
	SOCKET Accept()
	{
		// 4 accept �ȴ����ܿͻ�������
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
		if (INVALID_SOCKET == cSock)
		{
			printf("<socket=%d>���󣬽��ܵ���Ч�ͻ���SOCKET...\n", (int)_sock);
		}
		else
		{
			//NewUserJoin userJoin;
			//userJoin.sock = (int)cSock;
			//SendDataToAll(&userJoin);
			addClientToCellServer(new ClientSocket(cSock));
			//printf("<socket=%d>�¿ͻ���<%d>���룺socket = %d,IP = %s \n", (int)_sock, _clients.size(), (int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}

	void addClientToCellServer(ClientSocket* pClient)
	{
		_clients.push_back(pClient);
		//���ҿͻ��������ٵ�CellServer��Ϣ�������
		CellServer* pMinServer = _cellServers[0];
		for (CellServer* pCellServer : _cellServers)
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
			{
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);
	}

	void Start()
	{

		for (int n = 0; n < _CellServer_THREAD_COUNT; n++)
		{
			CellServer* ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			ser->setEventObj(this);
			ser->Start();
		}
	}
	//int _nCount = 0;
	// ����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			time4msg();
			//�������׽��� BSD socket
			fd_set fdRead;	//socket����
			fd_set fdWrite;
			fd_set fdExp;

			FD_ZERO(&fdRead);
			FD_SET(_sock, &fdRead);	//��socket���뵽socket����

			///ndfs��һ������ֵ����ָfd_set����������socket�ķ�Χ��������������
			///������socket���ֵ+1����windows��������������� 
			///��5��������NULL��������
			timeval t = { 0, 5000 };
			int ret = select(_sock + 1, &fdRead, nullptr, nullptr, &t);
			//printf("select ret=%d count=%d\n", ret, _nCount++);
			if (ret < 0) // �����쳣
			{
				printf("select�������\n");
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			return true;
		}
		Close();
		return false;
	}
	// ��Ӧ��������
	virtual void time4msg()
	{
		
		double t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			int recvCount = 0;
			for (CellServer* ser : _cellServers)
			{
				recvCount += ser->_recvCount;
				ser->_recvCount = 0;
			}
				
			printf("thread<%d>time<%1f>,socket<%d>,clients<%d>,_recvCount<%d>\n", _cellServers.size(),t1, (int)_sock, _clients.size(), (int)(recvCount / t1));
			recvCount = 0;
			_tTime.update();
		}
	}

	// ����ָ��socket����
	void SendData(SOCKET _cSock, DataHeader* header)
	{
		if (isRun() && header)
			send(_cSock, (const char*)header, header->dataLength, 0);
	}

	// Ⱥ��
	void SendDataToAll(DataHeader* header)
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			SendData(_clients[n]->sockfd(), header);
		}
	}

	// �ж�����״̬
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	// �ر�socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			// 8 �ر��׽���closesocket
			closesocket(_sock);
			//----------
			//���Windows socket���绷��
			WSACleanup();// �ر�windows socket���绷��
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
				delete _clients[n];
			}
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
			_clients.clear();
		}
	}
	
	virtual void OnLeave(ClientSocket* pclient)
	{	
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			if (_clients[n] == pclient)
			{
				std::vector<ClientSocket*>::iterator iter = _clients.begin() + n;
				if (iter != _clients.end())
					_clients.erase(iter);
			}
		}	
	}

	virtual void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		//time4msg();
	}

private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
	std::vector<CellServer*> _cellServers;
	//char _szRecv[RECV_BUFF_SIZE];
	CELLTimestamp _tTime;
	//int _recvCount;
};

#endif