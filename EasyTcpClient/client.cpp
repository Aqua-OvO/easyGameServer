#include<stdio.h>
#include<thread>
#include "EasyTcpClient.hpp" 

//#pragma comment(lib, "ws2_32.lib")
//WASStartup是引用的动态库，所以需要加上上面一行，引入动态库，ws2为WinSock2,32为32位
//但是这种写法只适用于windows平台下，所以应该在属性->链接器->输入->附加依赖项中添加这个库ws2_32.lib

bool g_bRun = true;

void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("退出cmdThread线程\n");
			break;
		}
		else
		{
			printf("不支持的命令。\n");
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

	//启动线程
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
		
		//线程thread
		//printf("空闲时间，处理其他业务中..\n");

	}
	for (int n = 0; n < cCount; n++)
	{
		client[n]->Close();
	}
	printf("任务结束，退出.");
	return 0;
}
