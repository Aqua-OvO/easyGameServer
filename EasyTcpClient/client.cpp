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

//�ͻ�������
const int cCount = 1000;
//�����߳�����
const int tCount = 4;

EasyTcpClient* client[cCount];

void sendThread(int id)
{
	int c = cCount / tCount;
	int begin = (id - 1) * c;
	int end = id * c;

	for (int n = begin; n < end; n++)
	{
		client[n] = new EasyTcpClient();
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->InitSocket();
		client[n]->Connect("127.0.0.1", 4567);
		printf("Connect=%d\n", n + 1);
	}

	//std::chrono::milliseconds t(3000);
	//std::this_thread::sleep_for(t);

	Login login;
	strcpy(login.userName, "sq");
	strcpy(login.passWord, "sq1234");
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			client[n]->OnRun();
			client[n]->SendData(&login);
		}

		//�߳�thread
		//printf("����ʱ�䣬��������ҵ����..\n");

	}
	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
	}
}

int main()
{
	//�����߳�
	std::thread t1(cmdThread);
	t1.detach();
	

	//���������߳�
	for (int n = 0; n < tCount; n++)
	{
		std::thread t1(sendThread, n+1);
		t1.detach();
	}

	while (g_bRun)
		Sleep(100);

	printf("����������˳�.");
	return 0;
}
