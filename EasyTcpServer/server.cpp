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

int main()
{
	EasyTcpServer server;
	server.InitSocket();
	server.Bind(NULL, 4567);
	server.Listen(5);
	server.Start();

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
