#ifndef EASY_TCP_CLIENT_HPP
#define EASY_TCP_CLIENT_HPP
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN //����windows��WinSock2�ض���
#include<windows.h>
#include<WinSock2.h>
#else
#include<unistd.h> //unix std
#include<arpa/inet.h> //��ӦWinSock2
#include<string.h>  //�ַ�������

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
	// ���ӷ�����
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
			printf("<socket=%d>�������ӷ�����<%s:%d>ʧ��...\n", (int)_sock, ip, port);
		}
		else
		{
			printf("<socket=%d>���ӷ�����<%s:%d>�ɹ�...\n", (int)_sock, ip, port);
		}
	}
	// �ر�socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			// �ر��׽���closesocket
			closesocket(_sock);
			//----------
			// ���Windows socket����
			WSACleanup();// �ر�windows socket���绷��
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		} 
	}
	//int _nCount = 0;
	// ��������
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
				printf("<socket=%d>select�������\n", (int)_sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData())
				{
					printf("<socket=%d>select�����쳣�˳�\n", (int)_sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}
	// �ж�����״̬
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	// ��������,����ճ������ְ�
#define RECV_BUFF_SIZE 10240
	// ���ջ�����
	char _szRecv[RECV_BUFF_SIZE] = {};
	// �ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	int _lastPos = 0;
	int RecvData()
	{
		// ���տͻ�������
		int nLen = (int)recv(_sock, _szRecv, RECV_BUFF_SIZE, 0);
		//printf("nLen=%d\n", nLen);
		if (nLen <= 0)
		{
			printf("<socket=%d>��������Ͽ�����,���������\n", (int)_sock);
			return -1;
		}
		// ���յ������ݿ�������Ϣ������
		memcpy(_szMsgBuf + _lastPos, _szRecv, nLen);
		// ��Ϣ������������β��λ�ú���
		_lastPos += nLen;
		// �ж���Ϣ�������ĳ����Ƿ������ϢͷDataHeader
		while (_lastPos >= sizeof(DataHeader))
		{
			DataHeader* header = (DataHeader*)_szMsgBuf;
			if (_lastPos >= header->dataLength)
			{
				// ʣ��δ������Ϣ���������ݳ���
				int nSize = _lastPos - header->dataLength;
				// ����������Ϣ
				OnNetMsg(header);
				// ����Ϣ������ʣ��δ��������ǰ��
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				// ��Ϣ������������β��λ��ǰ��
				_lastPos = nSize;
			}
			else
			{
				// ��Ϣ���������ݲ���һ��������Ϣ
				break;
			}
		}
		return 0;
	}

	// ��Ӧ
	void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* loginResult = (LoginResult*)header;
			//printf("<socket=%d>�յ���������Ϣ��CMD_LOGIN_RESULT ���ݳ��ȣ�%d\n", (int)_sock, loginResult->dataLength);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logoutResult = (LogoutResult*)header;
			//printf("<socket=%d>�յ���������Ϣ��CMD_LOGOUT_RESULT ���ݳ��ȣ�%d\n", (int)_sock, logoutResult->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* useJoin = (NewUserJoin*)header;
			//printf("<socket=%d>�յ���������Ϣ��CMD_NEW_USER_JOIN ���ݳ��ȣ�%d\n", (int)_sock, useJoin->dataLength);
		}
		break;
		default:
		{
			printf("<socket=%d>�յ�δ������Ϣ�����ݳ��ȣ�%d\n", (int)_sock, header->dataLength);
		}
		}
	}

	// ��������
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