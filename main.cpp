#include <iostream>
#include "Client.h"

CClient Client;

int main()
{
	printf_s("client start.\n");

	// 初始化Socket库
	if(!Client.LoadSocketLib())
	{
		printf_s("初始化socket库失败.\n");
		return 1;
	}

	Client.SetIP(DEFAULT_IP);
	Client.SetPort(DEFAULT_PORT);
	Client.SetThreads(DEFAULT_THREADS);
	Client.SetMessage(DEFAULT_MSG);

	// 开始
	if (!Client.Start())
	{
		printf_s("启动失败.\n");
		return 2;
	}

	printf_s("测试开始.\n");

	// 主线程停止循环
	string cmd = "";
	while (true)
	{
		printf_s(">");
		std::cin>>cmd;
		if ((cmd == "quit") || (cmd == "exit")) // 退出程序
		{
			Client.Stop(); // 关闭发送接口
			Client.UnloadSocketLib(); // 卸载socket库
			printf_s("exit server\n");
			break;
		}
	}

	printf_s("client end.\n");
	return 0;
}
