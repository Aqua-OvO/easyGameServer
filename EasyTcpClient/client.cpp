#define WIN32_LEAN_AND_MEAN //����windows��WinSock2�ض���
#include<windows.h>
#include<WinSock2.h>
#include<stdio.h>

//#pragma comment(lib, "ws2_32.lib")
//WASStartup�����õĶ�̬�⣬������Ҫ��������һ�У����붯̬�⣬ws2ΪWinSock2,32Ϊ32λ
//��������д��ֻ������windowsƽ̨�£�����Ӧ��������->������->����->��������������������ws2_32.lib

//
enum CMD
{
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_ERROR
};

struct DataHeader
{
	short dataLength;	//���ݳ���
	short cmd;			//����
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
	//����windows socket 2.x����
	WSAStartup(ver, &dat);
	//---------
	//-- ��Socket API��������TCP�ͻ���
	// 1 ����һ��socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		printf("���󣬽���Socketʧ��...\n");
	}
	else
	{
		printf("����Socket�ɹ�...\n");
	}
	// 2 ���ӷ����� connect
	sockaddr_in _sin = {};
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	_sin.sin_port = htons(4567);
	_sin.sin_family = AF_INET;

	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("��������Socketʧ��...\n");
	}
	else
	{
		printf("����Socket�ɹ�...\n");
	}
	
	while (true)
	{
		// 3 ������������
		char cmdBuf[128] = {};
		scanf("%s", cmdBuf);
		// 4 ������������
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("�յ�exit����\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login = {"sq", "sqmima"};
			DataHeader dh = {sizeof(Login), CMD_LOGIN};
			// 5 �������������������
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
			printf("��֧�ֵ�������������롣 \n");
		}
	}


	// 7 �ر��׽���closesocket
	closesocket(_sock);
	//----------
	//���Windows socket����
	WSACleanup();// �ر�windows socket���绷��
	printf("����������Ƴ�.");
	getchar();
	return 0;
}