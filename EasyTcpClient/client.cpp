#define WIN32_LEAN_AND_MEAN //����windows��WinSock2�ض���
#include<windows.h>
#include<WinSock2.h>
#include<stdio.h>

//#pragma comment(lib, "ws2_32.lib")
//WASStartup�����õĶ�̬�⣬������Ҫ��������һ�У����붯̬�⣬ws2ΪWinSock2,32Ϊ32λ
//��������д��ֻ������windowsƽ̨�£�����Ӧ��������->������->����->���������������������ws2_32.lib

struct DataPackage
{
	int age;
	char name[32];
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
		else
		{
			// 5 �������������������
			send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
		}

		// 6 ���ܷ�������Ϣrecv
		char recvBuf[256] = {};
		int nLen = recv(_sock, recvBuf, 256, 0);
		if (nLen > 0) {
			DataPackage* dp = (DataPackage*)recvBuf;
			printf("���յ����ݣ�����=%d,����=%s \n", dp->age, dp->name);
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