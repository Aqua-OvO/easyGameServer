#define WIN32_LEAN_AND_MEAN //避免windows和WinSock2重定义
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<windows.h>
#include<WinSock2.h>
#include<stdio.h>

//#pragma comment(lib, "ws2_32.lib")
//WASStartup是引用的动态库，所以需要加上上面一行，引入动态库，ws2为WinSock2,32为32位
//但是这种写法只适用于windows平台下，所以应该在属性->链接器->输入->附加依赖项中添加这个库ws2_32.lib

struct DataPackage
{
	int age;
	char name[32];
};

int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	//启动windows socket 2.x环境
	WSAStartup(ver, &dat);
	//---------

	//-- 用Socket API建立简易TCP服务端
	// 1 建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// 2 bind 绑定用于接受客户端连接的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567); // host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; // 内网地址或回环地址都可，inet_addr("127.0.0.1"); 
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("错误,绑定网络端口失败...\n");
	}
	else
	{
		printf("绑定网络端口成功...\n");
	}
	// 3 listen 监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("错误,监听网络端口失败...\n");
	}
	else
	{
		printf("监听网络端口成功...\n");
	}
	// 4 accept 等待接受客户端连接
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;
	//char msgBuf[] = "hello, I'm Server.";
	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock)
	{
		printf("错误，接受到无效客户端SOCKET...\n");
	}
	printf("新客户端加入：socket = %d,IP = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));

	char _recvBuf[128] = {};
	while (true)
	{
		// 5 接收客户端数据
		int nLen = recv(_cSock, _recvBuf, 128, 0);
		if (nLen <= 0)
		{
			printf("客户端已退出，任务结束。\n");
			break;
		}
		printf("收到命令：%s \n", _recvBuf);
		// 6 处理请求
		if (0 == strcmp(_recvBuf, "getInfo"))
		{
			DataPackage dp = { 80, "张国荣" };
			// 7 send 向客户端发送数据
			send(_cSock, (const char*)&dp, sizeof(DataPackage) + 1, 0);
		}
		else {
			char msgBuf[] = "???.";
			// 7 send 向客户端发送数据
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
	}
	// 8 关闭套接字closesocket
	closesocket(_sock);
	//----------
	//清除Windows socket环境
	WSACleanup();// 关闭windows socket网络环境
	printf("任务结束，推出.");
	getchar();
	return 0;
}