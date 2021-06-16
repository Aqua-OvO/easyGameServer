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
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

struct DataHeader
{
	short dataLength;	//数据长度
	short cmd;			//命令
};
// DataPackage
struct Login : public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};

struct LoginResult : public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Logout : public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult : public DataHeader
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};

int processor(SOCKET _cSock)
{
	// 缓冲区
	char szRecv[4096];
	// 5 接收客户端数据
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("与服务器断开连接,任务结束。\n");
		return -1;
	}
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* loginResult = (LoginResult*)szRecv;
		printf("收到服务器消息：CMD_LOGIN_RESULT 数据长度：%d\n", loginResult->dataLength);
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LogoutResult* logoutResult = (LogoutResult*)szRecv;
		printf("收到服务器消息：CMD_LOGOUT_RESULT 数据长度：%d\n", logoutResult->dataLength);
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoin* useJoin = (NewUserJoin*)szRecv;
		printf("收到服务器消息：CMD_NEW_USER_JOIN 数据长度：%d\n", useJoin->dataLength);
	}
	break;
	default:
		DataHeader header = { 0,CMD_ERROR };
		send(_cSock, (char*)&header, sizeof(header), 0);
		break;
	}
	return 0;
}

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
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t = { 0, 1000000 };
		int ret = select(_sock + 1, &fdReads, NULL, NULL, &t);
		if (ret < 0)
		{
			printf("select任务结束\n");
			break;
		}
		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);
			if (-1 == processor(_sock)) 
			{
				printf("select任务异常退出\n");
				break;
			}
		}
		printf("空闲时间，处理其他业务中..\n");
		Login login;
		strcpy(login.userName, "sq");
		strcpy(login.passWord, "mima1234");
		send(_sock, (const char *)&login, sizeof(login), 0);
		Sleep(1000);
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