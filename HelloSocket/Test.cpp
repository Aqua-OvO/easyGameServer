#define WIN32_LEAN_AND_MEAN //避免windows和WinSock2重定义
#include<windows.h>
#include<WinSock2.h>


//#pragma comment(lib, "ws2_32.lib")
//WASStartup是引用的动态库，所以需要加上上面一行，引入动态库，ws2为WinSock2,32为32位
//但是这种写法只适用于windows平台下，所以应该在属性->链接器->输入->附加依赖项中添加这个库ws2_32.lib

int main()
{
	WORD ver = MAKEWORD(2,2);
	WSADATA dat;
	//启动windows socket 2.x环境
	WSAStartup(ver, &dat);
	//---------
	//-- 用Socket API建立简易TCP客户端
	// 1 建立一个socket
	// 2 连接服务器 connect
	// 3 接受服务器信息recv
	// 4 关闭套接字closesocket
	//-- 用Socket API建立简易TCP服务端
	// 1 建立一个socket
	// 2 bind 绑定用于接受客户端连接的网络端口
	// 3 listen 监听网络端口
	// 4 accept 等待接受客户端连接
	// 5 send 向客户端发送一条数据
	// 6 关闭套接字closesocket
	//----------
	//清除Windows socket环境
	WSACleanup();// 关闭windows socket网络环境
	return 0;
}