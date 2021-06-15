#define WIN32_LEAN_AND_MEAN //避免windows和WinSock2重定义
#include<windows.h>
#include<WinSock2.h>
#include<stdio.h>

//#pragma comment(lib, "ws2_32.lib")
//WASStartup是引用的动态库，所以需要加上上面一行，引入动态库，ws2为WinSock2,32为32位
//但是这种写法只适用于windows平台下，所以应该在属性->链接器->输入->附加依赖项中添加这个库ws2_32.lib

//
enum CMD
{
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_ERROR
};

struct DataHeader
{
	short dataLength;	//数据长度
	short cmd;			//命令
};
// DataPackage
struct Login
{
	char userName[32];
	char passWord[32];
};

struct LoginResult
{
	int result;
};

struct Logout
{
	char userName[32];
};

struct LogoutResult
{
	int result;
};


int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	//启动windows socket 2.x环境
	WSAStartup(ver, &dat);
	//---------
	//-- 用Socket API建立简易TCP客户端
	// 1 建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		printf("错误，建立Socket失败...\n");
	}
	else
	{
		printf("建立Socket成功...\n");
	}
	// 2 连接服务器 connect
	sockaddr_in _sin = {};
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	_sin.sin_port = htons(4567);
	_sin.sin_family = AF_INET;

	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("错误，连接Socket失败...\n");
	}
	else
	{
		printf("连接Socket成功...\n");
	}
	
	while (true)
	{
		// 3 输入请求命令
		char cmdBuf[128] = {};
		scanf("%s", cmdBuf);
		// 4 处理请求命令
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("收到exit命令\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login = {"sq", "sqmima"};
			DataHeader dh = {sizeof(Login), CMD_LOGIN};
			// 5 向服务器发送请求命令
			send(_sock, (const char *)&dh, sizeof(dh), 0);
			send(_sock, (const char *)&login, sizeof(login), 0);
			DataHeader retHeader = {};
			LoginResult loginRet = {};
			recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult: %d\n", loginRet.result);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout = {"sq"};
			DataHeader dh = { sizeof(Logout), CMD_LOGOUT };

			send(_sock, (const char *)&dh, sizeof(dh), 0);
			send(_sock, (const char *)&logout, sizeof(logout), 0);
			DataHeader retHeader = {};
			LoginResult logoutRet = {};
			recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&logoutRet, sizeof(logoutRet), 0);
			printf("LogoutResult: %d\n", logoutRet.result);
		}
		else
		{
			printf("不支持的命令，请重新输入。 \n");
		}
	}


	// 7 关闭套接字closesocket
	closesocket(_sock);
	//----------
	//清除Windows socket环境
	WSACleanup();// 关闭windows socket网络环境
	printf("任务结束，推出.");
	getchar();
	return 0;
}