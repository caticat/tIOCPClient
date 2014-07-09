#pragma once

#include <string>
#include <WinSock2.h>

#define MAX_BUFFER_LEN 8196 // 缓冲区长度1024*8
#define DEFAULT_PORT 12345
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_THREADS 100
#define DEFAULT_MSG "HELLO!"

using std::string;

class CClient;

// 用于发送数据的线程参数
struct THREADPARAMS_WORKER
{
	CClient* pClient; // 类指针
	SOCKET sock; // 每个线程使用的socket
	int nThreadNo; // 线程编号
	char szBuffer[MAX_BUFFER_LEN];
};

// 产生socket连接的线程参数
struct THREADPARAMS_CONNECTION
{
	CClient* pClient;
};

class CClient
{
public:
	CClient();
	~CClient();

public:
	bool LoadSocketLib(); // 加载socket库
	void UnloadSocketLib() { WSACleanup(); } // 卸载socket库

	bool Start(); // 开始测试
	void Stop(); // 停止测试

	string GetLocalIP(); // 获得本机IP地址

	void SetIP(const string& strIP) { m_strServerIP = strIP; } // 设置服务器IP
	void SetPort(const int& nPort) { m_nPort = nPort; } // 设置连接服务器端口
	void SetThreads(const int& n) { m_nThreads = n; } // 设置并发线程数量
	void SetMessage(const string& strMessage) { m_strMessage = strMessage; } // 设置发送的消息

private:
	bool EstablishConnections(); // 建立连接
	bool ConnectToServer(SOCKET* pSocket,string strServer,int nPort); // 向服务器进行连接
	static DWORD WINAPI _ConnectionThread(LPVOID lpParam); // 用于建立连接的线程
	static DWORD WINAPI _WorkerThread(LPVOID lpParam); // 用于发送信息的线程

	void CleanUp(); // 释放资源

private:
	string m_strServerIP; // 服务器的IP地址
	string m_strLocalIP; // 本机IP地址
	string m_strMessage; // 发送的消息
	int m_nPort; // 服务器端口
	int m_nThreads; // 并发线程数量

	HANDLE* m_phWorkerThreads; // 工作线程句柄数组
	HANDLE m_hConnectionThread; // 连接线程的句柄
	HANDLE m_hShutdownEvent; // 系统停止退出事件

	THREADPARAMS_WORKER* m_pParamsWorker; // 线程参数
};
