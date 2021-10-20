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

//客户端数量
const int cCount = 10000;
//发送线程数量
const int tCount = 4;

EasyTcpClient* client[cCount];

void sendThread(int id)
{
	printf("thread<%d>,start\n", id);
	int c = cCount / tCount;
	int begin = (id - 1) * c;
	int end = id * c;

	for (int n = begin; n < end; n++)
	{
		client[n] = new EasyTcpClient();
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Connect("127.0.0.1", 4567);
	}
	printf("thread<%d>,Connect<begin=%d, end=%d>\n", id, begin, end);
	//std::chrono::milliseconds t(3000);
	//std::this_thread::sleep_for(t);

	Login login[10];
	for (int n = 0; n < 10; n++)
	{
		strcpy(login[n].userName, "sq");
		strcpy(login[n].passWord, "sq1234");
	}
	const int nLen = sizeof(login);
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			client[n]->SendData(login, nLen);
			client[n]->OnRun();
		}

		//线程thread
		//printf("空闲时间，处理其他业务中..\n");

	}
	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
		delete client[n];
	}
	printf("thread<%d>,exit\n", id);
}

int main()
{
	//启动线程
	std::thread t1(cmdThread);
	t1.detach();
	

	//启动发送线程
	for (int n = 0; n < tCount; n++)
	{
		std::thread t1(sendThread, n+1);
		t1.detach();
	}

	while (g_bRun)
		Sleep(100);

	printf("任务结束，退出.");
	return 0;
}
