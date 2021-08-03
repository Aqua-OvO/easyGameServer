#include<stdio.h>
#include<vector>
#include<thread>
#include "EasyTcpServer.hpp"

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
			g_bRun = false;
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
		//printf("空闲时间，处理其他业务中..\n");
	}
	server.Close();
	printf("任务结束，退出.\n");
	getchar();
	return 0;
}
