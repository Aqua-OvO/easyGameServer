#include<stdio.h>
#include<thread>
#include "EasyTcpClient.hpp" 

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
	const int cCount = 2000;
	EasyTcpClient* client[cCount];
	for (int n = 0; n < cCount; n++)
	{
		if (!g_bRun)
			return 0;
		client[n] = new EasyTcpClient();
		if (!g_bRun)
			return 0;
		client[n]->InitSocket();
		if (!g_bRun)
			return 0;
		client[n]->Connect("127.0.0.1", 4567);
		printf("Connect=%d\n", n + 1);
	}

	//�����߳�
	std::thread t1(cmdThread);
	t1.detach();
	Login login;
	strcpy(login.userName, "sq");
	strcpy(login.passWord, "sq1234");
	while (g_bRun)
	{
		for (int n = 0; n < cCount; n++)
		{
			client[n]->OnRun();
			client[n]->SendData(&login);
		}
		
		//�߳�thread
		//printf("����ʱ�䣬��������ҵ����..\n");

	}
	for (int n = 0; n < cCount; n++)
	{
		client[n]->Close();
	}
	printf("����������˳�.");
	return 0;
}
