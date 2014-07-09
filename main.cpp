#include <iostream>
#include "Client.h"

CClient Client;

int main()
{
	printf_s("client start.\n");

	// ��ʼ��Socket��
	if(!Client.LoadSocketLib())
	{
		printf_s("��ʼ��socket��ʧ��.\n");
		return 1;
	}

	Client.SetIP(DEFAULT_IP);
	Client.SetPort(DEFAULT_PORT);
	Client.SetThreads(DEFAULT_THREADS);
	Client.SetMessage(DEFAULT_MSG);

	// ��ʼ
	if (!Client.Start())
	{
		printf_s("����ʧ��.\n");
		return 2;
	}

	printf_s("���Կ�ʼ.\n");

	// ���߳�ֹͣѭ��
	string cmd = "";
	while (true)
	{
		printf_s(">");
		std::cin>>cmd;
		if ((cmd == "quit") || (cmd == "exit")) // �˳�����
		{
			Client.Stop(); // �رշ��ͽӿ�
			Client.UnloadSocketLib(); // ж��socket��
			printf_s("exit server\n");
			break;
		}
	}

	printf_s("client end.\n");
	return 0;
}
