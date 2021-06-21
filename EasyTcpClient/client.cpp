#include<stdio.h>
#include<thread>
#include "EasyTcpClient.hpp" 

//#pragma comment(lib, "ws2_32.lib")
//WASStartup是引用的动态库，所以需要加上上面一行，引入动态库，ws2为WinSock2,32为32位
//但是这种写法只适用于windows平台下，所以应该在属性->链接器->输入->附加依赖项中添加这个库ws2_32.lib

void cmdThread(EasyTcpClient* client)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("退出cmdThread线程\n");
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
			printf("不支持的命令。\n");
		}
	}
}

int main()
{
	EasyTcpClient client;
	client.InitSocket();
	client.Connect("127.0.0.1", 4567);

	//启动线程
	std::thread t1(cmdThread, &client);
	t1.detach();
	Login login;
	strcpy(login.userName, "sq");
	strcpy(login.passWord, "sq1234");
	while (client.isRun())
	{
		client.OnRun();
		client.SendData(&login);
		//线程thread
		//printf("空闲时间，处理其他业务中..\n");

	}
	client.Close();
	printf("任务结束，推出.");
	getchar();
	return 0;
}
