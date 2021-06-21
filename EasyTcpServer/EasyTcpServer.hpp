#ifndef EASY_TCP_SERVER_HPP
#define EASY_TCP_SERVER_HPP

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN //避免windows和WinSock2重定义
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
#include<stdio.h>
#include<vector>
#include "MessageHeader.hpp"

class EasyTcpServer
{
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}

	virtual ~EasyTcpServer()
	{
		Close();
	}
	// 初始化socket
	void InitSocket()
	{
#ifdef _WIN32
		//启动windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("关闭旧连接<socket=%d>..\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("错误，建立Socket失败...\n");
		}
		else
		{
			printf("<socket=%d>建立成功...\n", (int)_sock);
		}
	}
	// 绑定端口号
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
			_sin.sin_addr.S_un.S_addr = INADDR_ANY; // 内网地址或回环地址都可，inet_addr("127.0.0.1"); 
#else
		if (ip)
			_sin.sin_addr.s_addr = inet_addr(ip);
		else
			_sin.sin_addr.s_addr = INADDR_ANY; // 内网地址或回环地址都可，inet_addr("127.0.0.1"); 
#endif    
		if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
		{
			printf("<socket=%d>错误,绑定网络端口<%d>失败...\n", (int)_sock, port);
		}
		else
		{
			printf("<socket=%d>绑定网络端口<%d>成功...\n", (int)_sock, port);
		}
		return _sock;
	}
	// 监听端口号
	int Listen(int n)
	{
		// listen 监听网络端口
		int ret = listen(_sock, 5);
		if (SOCKET_ERROR == ret)
		{
			printf("<socket=%d>错误,监听网络端口失败...\n", (int)_sock);
		}
		else
		{
			printf("<socket=%d>监听网络端口成功...\n", (int)_sock);
		}
		return ret;
	}
	// 接受客户端连接
	SOCKET Accept()
	{
		// 4 accept 等待接受客户端连接
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
		//char msgBuf[] = "hello, I'm Server.";
		_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
		if (INVALID_SOCKET == _cSock)
		{
			printf("<socket=%d>错误，接受到无效客户端SOCKET...\n", (int)_sock);
		}
		else
		{
			NewUserJoin userJoin;
			userJoin.sock = (int)_cSock;
			SendDataToAll(&userJoin);
			g_clients.push_back(_cSock);
			printf("<socket=%d>新客户端加入：socket = %d,IP = %s \n", (int)_sock, (int)_cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return _cSock;
	}
	// 处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			//伯克利套接字 BSD socket
			fd_set fdRead;	//socket集合
			fd_set fdWrite;
			fd_set fdExp;

			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			FD_SET(_sock, &fdRead);	//将socket加入到socket集合
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdWrite);
			SOCKET maxSock = _sock;
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(g_clients[n], &fdRead);
				if (maxSock < g_clients[n])
				{
					maxSock = g_clients[n];
				}
			}
			///ndfs是一个整数值，是指fd_set集合中所有socket的范围，而不是数量，
			///即所有socket最大值+1，在windows中这个参数无作用 
			///第5个参数传NULL则变成阻塞
			timeval t = { 0, 0 };
			int ret = select((int)maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0) // 发生异常
			{
				printf("select任务结束\n");
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(g_clients[n], &fdRead))
				{
					if (-1 == RecvData(g_clients[n]))
					{
						auto iter = g_clients.begin() + n;
						if (iter != g_clients.end())
						{
							g_clients.erase(iter);
						}
					}
				}
			}
			return true;
		}
		Close();
		return false;
	}
	char szRecv[409600];
	// 接收数据
	int RecvData(SOCKET _cSock)
	{
		// 缓冲区
		int nLen = (int)recv(_cSock, szRecv, 409600, 0);
		printf("nLen=%d\n", nLen);
		LoginResult ret;
		SendData(_cSock, &ret);
		/*
		char szRecv[4096];
		// 5 接收客户端数据
		int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
			printf("客户端<Socket=%d>已退出，任务结束。\n", (int)_cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(_cSock, header);
		*/
		return 0;
	}
	// 响应网络数据
	virtual void OnNetMsg(SOCKET _cSock, DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			printf("收到客户端<Socket=%d>请求：CMD_LOGIN 数据长度：%d, userName=%s PassWord=%s\n", (int)_cSock, login->dataLength, login->userName, login->passWord);
			//忽略判断用户密码是否正确的过程
			LoginResult ret;
			SendData(_cSock, &ret);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			printf("收到客户端<Socket=%d>请求：CMD_LOGOUT 数据长度：%d, userName=%s\n", (int)_cSock, logout->dataLength, logout->userName);
			//忽略判断用户密码是否正确的过程
			LogoutResult ret;
			SendData(_cSock, &ret);
		}
		break;
		default:
			DataHeader header = { 0,CMD_ERROR };
			SendData(_cSock, &header);
			break;
		}
	}

	// 发送指定socket数据
	void SendData(SOCKET _cSock, DataHeader* header)
	{
		if (isRun() && header)
			send(_cSock, (const char*)header, header->dataLength, 0);
	}

	// 群发

	void SendDataToAll(DataHeader* header)
	{
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			SendData(g_clients[n], header);
		}
	}

	// 判断运行状态
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	// 关闭socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				closesocket(g_clients[n]);
			}
			// 8 关闭套接字closesocket
			closesocket(_sock);
			//----------
			//清除Windows socket环境
			WSACleanup();// 关闭windows socket网络环境
#else
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]);
			}
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
	}
private:
	SOCKET _sock;
	std::vector<SOCKET> g_clients;
};

#endif