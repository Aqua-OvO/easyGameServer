#include<stdio.h>
#include<vector>
#include<thread>
#include "EasyTcpServer.hpp"

//#pragma comment(lib, "ws2_32.lib")
//WASStartup�����õĶ�̬�⣬������Ҫ��������һ�У����붯̬�⣬ws2ΪWinSock2,32Ϊ32λ
//��������д��ֻ������windowsƽ̨�£�����Ӧ��������->������->����->��������������������ws2_32.lib

bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("�˳�cmdThread�߳�\n");
			g_bRun = false;
			break;
		}
		else
		{
			printf("��֧�ֵ����\n");
		}
	}
}

//class MyServer : public EasyTcpServer
//{
//public:
//	virtual void OnNetMsg(ClientSocket* pclient, DataHeader* header)
//	{
//		switch (header->cmd)
//		{
//		case CMD_LOGIN:
//		{
//			Login* login = (Login*)header;
//			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGIN ���ݳ��ȣ�%d, userName=%s PassWord=%s\n", (int)cSock, login->dataLength, login->userName, login->passWord);
//			//�����ж��û������Ƿ���ȷ�Ĺ���
//			LoginResult ret;
//			pclient->SendData(&ret);
//		}
//		break;
//		case CMD_LOGOUT:
//		{
//			Logout* logout = (Logout*)header;
//			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGOUT ���ݳ��ȣ�%d, userName=%s\n", (int)cSock, logout->dataLength, logout->userName);
//			//�����ж��û������Ƿ���ȷ�Ĺ���
//			LogoutResult ret;
//			pclient->SendData(&ret);
//		}
//		break;
//		default:
//			printf("<socket=%d>�յ�δ������Ϣ�����ݳ��ȣ�%d\n", (int)pclient->sockfd(), header->dataLength);
//			// DataHeader header;
//			// SendData(cSock, &header);
//			break;
//		}
//	}
//
//private:
//
//};

int main()
{
	EasyTcpServer server;
	server.InitSocket();
	server.Bind(NULL, 4567);
	server.Listen(5);
	server.Start(4);

	std::thread t1(cmdThread);
	t1.detach();
	while (g_bRun)
	{
		server.OnRun();
		//printf("����ʱ�䣬��������ҵ����..\n");
	}
	server.Close();
	printf("����������˳�.\n");
	getchar();
	return 0;
}
