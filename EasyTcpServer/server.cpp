#include<stdio.h>
#include<vector>
#include "EasyTcpServer.hpp"

//#pragma comment(lib, "ws2_32.lib")
//WASStartup�����õĶ�̬�⣬������Ҫ��������һ�У����붯̬�⣬ws2ΪWinSock2,32Ϊ32λ
//��������д��ֻ������windowsƽ̨�£�����Ӧ��������->������->����->��������������������ws2_32.lib

int main()
{
	EasyTcpServer server;
	server.InitSocket();
	server.Bind(NULL, 4567);
	server.Listen(5);
	while (server.isRun())
	{
		server.OnRun();
		//printf("����ʱ�䣬��������ҵ����..\n");
	}
	server.Close();
	printf("����������Ƴ�.\n");
	getchar();
	return 0;
}
