#define WIN32_LEAN_AND_MEAN //����windows��WinSock2�ض���
#include<windows.h>
#include<WinSock2.h>


//#pragma comment(lib, "ws2_32.lib")
//WASStartup�����õĶ�̬�⣬������Ҫ��������һ�У����붯̬�⣬ws2ΪWinSock2,32Ϊ32λ
//��������д��ֻ������windowsƽ̨�£�����Ӧ��������->������->����->��������������������ws2_32.lib

int main()
{
	WORD ver = MAKEWORD(2,2);
	WSADATA dat;
	//����windows socket 2.x����
	WSAStartup(ver, &dat);
	//---------
	//-- ��Socket API��������TCP�ͻ���
	// 1 ����һ��socket
	// 2 ���ӷ����� connect
	// 3 ���ܷ�������Ϣrecv
	// 4 �ر��׽���closesocket
	//-- ��Socket API��������TCP�����
	// 1 ����һ��socket
	// 2 bind �����ڽ��ܿͻ������ӵ�����˿�
	// 3 listen ��������˿�
	// 4 accept �ȴ����ܿͻ�������
	// 5 send ��ͻ��˷���һ������
	// 6 �ر��׽���closesocket
	//----------
	//���Windows socket����
	WSACleanup();// �ر�windows socket���绷��
	return 0;
}