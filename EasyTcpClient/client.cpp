#include<stdio.h>
#include<thread>
#include "EasyTcpClient.hpp" 

//#pragma comment(lib, "ws2_32.lib")
//WASStartup�����õĶ�̬�⣬������Ҫ��������һ�У����붯̬�⣬ws2ΪWinSock2,32Ϊ32λ
//��������д��ֻ������windowsƽ̨�£�����Ӧ��������->������->����->��������������������ws2_32.lib

void cmdThread(EasyTcpClient* client)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("�˳�cmdThread�߳�\n");
			client->Close();
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "sq");
			strcpy(login.passWord, "mima1234");
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "sq");
			client->SendData(&logout);
		}
		else
		{
			printf("��֧�ֵ����\n");
		}
	}
}

int main()
{
	EasyTcpClient client;
	client.InitSocket();
	client.Connect("127.0.0.1", 4567);

	//�����߳�
	std::thread t1(cmdThread, &client);
	t1.detach();
	Login login;
	strcpy(login.userName, "sq");
	strcpy(login.passWord, "sq1234");
	while (client.isRun())
	{
		client.OnRun();
		client.SendData(&login);
		//�߳�thread
		//printf("����ʱ�䣬��������ҵ����..\n");

	}
	client.Close();
	printf("����������Ƴ�.");
	getchar();
	return 0;
}
