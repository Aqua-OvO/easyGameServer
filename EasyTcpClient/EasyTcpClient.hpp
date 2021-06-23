#ifndef EASY_TCP_CLIENT_HPP
#define EASY_TCP_CLIENT_HPP
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN //避免windows和WinSock2重定义
#include<windows.h>
#include<WinSock2.h>
#else
#include<unistd.h> //unix std
#include<arpa/inet.h> //对应WinSock2
#include<string.h>  //字符串操作

#define SOCKET int
#define INVALID_SOCKET (SOCKET)(-0)
#define SOCKET_ERROR           (-1)
#endif

#include <stdio.h>
#include "MessageHeader.hpp"

class EasyTcpClient
{
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}

	virtual ~EasyTcpClient()
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
	// 连接服务器
	void Connect(const char* ip, short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		sockaddr_in _sin = {};
#ifdef _WIN32    
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif    
		_sin.sin_port = htons(port);
		_sin.sin_family = AF_INET;

		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("<socket=%d>错误，连接服务器<%s:%d>失败...\n", (int)_sock, ip, port);
		}
		else
		{
			printf("<socket=%d>连接服务器<%s:%d>成功...\n", (int)_sock, ip, port);
		}
	}
	// 关闭socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			// 关闭套接字closesocket
			closesocket(_sock);
			//----------
			// 清除Windows socket环境
			WSACleanup();// 关闭windows socket网络环境
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		} 
	}
	//int _nCount = 0;
	// 处理数据
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 0, 100000 };
			int ret = select((int)_sock + 1, &fdReads, NULL, NULL, &t);
			//printf("select ret=%d count=%d\n", ret, _nCount++);
			if (ret < 0)
			{
				printf("<socket=%d>select任务结束\n", (int)_sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData())
				{
					printf("<socket=%d>select任务异常退出\n", (int)_sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}
	// 判断运行状态
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	// 接收数据,处理粘包、拆分包
#define RECV_BUFF_SIZE 10240
	// 接收缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};
	// 第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	int _lastPos = 0;
	int RecvData()
	{
		// 接收客户端数据
		int nLen = (int)recv(_sock, _szRecv, RECV_BUFF_SIZE, 0);
		//printf("nLen=%d\n", nLen);
		if (nLen <= 0)
		{
			printf("<socket=%d>与服务器断开连接,任务结束。\n", (int)_sock);
			return -1;
		}
		// 将收到的数据拷贝到消息缓冲区
		memcpy(_szMsgBuf + _lastPos, _szRecv, nLen);
		// 消息缓冲区的数据尾部位置后移
		_lastPos += nLen;
		// 判断消息缓冲区的长度是否大于消息头DataHeader
		while (_lastPos >= sizeof(DataHeader))
		{
			DataHeader* header = (DataHeader*)_szMsgBuf;
			if (_lastPos >= header->dataLength)
			{
				// 剩余未处理消息缓冲区数据长度
				int nSize = _lastPos - header->dataLength;
				// 处理网络消息
				OnNetMsg(header);
				// 将消息缓冲区剩余未处理数据前移
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				// 消息缓冲区的数据尾部位置前移
				_lastPos = nSize;
			}
			else
			{
				// 消息缓冲区数据不够一条完整消息
				break;
			}
		}
		return 0;
	}

	// 响应
	void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* loginResult = (LoginResult*)header;
			//printf("<socket=%d>收到服务器消息：CMD_LOGIN_RESULT 数据长度：%d\n", (int)_sock, loginResult->dataLength);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logoutResult = (LogoutResult*)header;
			//printf("<socket=%d>收到服务器消息：CMD_LOGOUT_RESULT 数据长度：%d\n", (int)_sock, logoutResult->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* useJoin = (NewUserJoin*)header;
			//printf("<socket=%d>收到服务器消息：CMD_NEW_USER_JOIN 数据长度：%d\n", (int)_sock, useJoin->dataLength);
		}
		break;
		default:
		{
			printf("<socket=%d>收到未定义消息，数据长度：%d\n", (int)_sock, header->dataLength);
		}
		}
	}

	// 发送数据
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
			send(_sock, (char*)header, header->dataLength, 0);

		return SOCKET_ERROR;
	}

private:
	SOCKET _sock;
};

#endif