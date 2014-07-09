#include "Client.h"
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

#define RELEASE_HANDLE(x) {if(x != NULL && x != INVALID_HANDLE_VALUE){ CloseHandle(x); x = NULL; }}
#define RELEASE(x) {if(x != NULL){ delete x; x = NULL; }}
#define RELEASE_ARR(x) {if(x != NULL){ delete[] x; x = NULL; }}

CClient::CClient():
				m_strServerIP(DEFAULT_IP),
				m_strLocalIP(DEFAULT_IP),
				m_nThreads(DEFAULT_THREADS),
				m_nPort(DEFAULT_PORT),
				m_strMessage(DEFAULT_MSG),
				m_phWorkerThreads(NULL),
				m_hConnectionThread(NULL),
				m_hShutdownEvent(NULL)
{

}

CClient::~CClient()
{
	this->Stop();
}

// 建立连接的线程
DWORD WINAPI CClient::_ConnectionThread(LPVOID lpParam)
{
	THREADPARAMS_CONNECTION* pParams = (THREADPARAMS_CONNECTION*)lpParam;
	CClient* pClient = pParams->pClient;

	printf_s("_AcceptThread启动,系统监听中.\n");
	pClient->EstablishConnections();
	printf_s("_ConnectionThread线程结束.\n");

	RELEASE(pParams);
	return 0;
}

// 用于发送信息的线程
DWORD WINAPI CClient::_WorkerThread(LPVOID lpParam)
{
	THREADPARAMS_WORKER* pParams = (THREADPARAMS_WORKER*)lpParam;
	CClient* pClient = pParams->pClient;

	char szTemp[MAX_BUFFER_LEN] = {0};
	char szRecv[MAX_BUFFER_LEN] = {0};

	int nByteSent = 0;
	int nByteRecv = 0;

	// 向服务器发送第一条信息
	sprintf_s(szTemp,sizeof(szTemp),"第1条信息：%s",pParams->szBuffer);
	nByteSent = send(pParams->sock,szTemp,strlen(szTemp),0);
	if (SOCKET_ERROR == nByteSent)
	{
		printf_s("错误：第1次发送信息失败，错误码：%d.\n",WSAGetLastError());
		return 1;
	}
	printf_s("向服务器发送信息成功：%s.\n",szTemp);
	Sleep(3000);

	// 向服务器发送第二条信息
	memset(szTemp,0,sizeof(szTemp));
	sprintf_s(szTemp,sizeof(szTemp),"第2条信息：%s",pParams->szBuffer);
	nByteSent = send(pParams->sock,szTemp,strlen(szTemp),0);
	if (SOCKET_ERROR == nByteSent)
	{
		printf_s("错误：第2次发送信息失败，错误码：%d.\n",WSAGetLastError());
		return 1;
	}
	printf_s("向服务器发送信息成功：%s.\n",szTemp);
	Sleep(3000);

	// 向服务器发送第三条信息
	memset(szTemp,0,sizeof(szTemp));
	sprintf_s(szTemp,sizeof(szTemp),"第3条信息：%s",pParams->szBuffer);
	nByteSent = send(pParams->sock,szTemp,strlen(szTemp),0);
	if (SOCKET_ERROR == nByteSent)
	{
		printf_s("错误：第3次发送信息失败，错误码：%d.\n",WSAGetLastError());
		return 1;
	}
	printf_s("向服务器发送信息成功：%s.\n",szTemp);

	if (pParams->nThreadNo == pClient->m_nThreads) // 这里感觉不对
	{
		printf_s("测试并发%d个线程完毕.\n",pClient->m_nThreads);
	}
	return 0;
}

// 建立连接
bool CClient::EstablishConnections()
{
	m_phWorkerThreads = new HANDLE[m_nThreads];
	m_pParamsWorker = new THREADPARAMS_WORKER[m_nThreads];

	for (int i = 0; i < m_nThreads; ++i)
	{
		// 监听用户停止事件
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_hShutdownEvent,0))
		{
			printf_s("接收到用户停止命令.\n");
			return true;
		}

		// 连接服务器
		if (!this->ConnectToServer(&m_pParamsWorker[i].sock,m_strServerIP,m_nPort))
		{
			printf_s("连接服务器失败.\n");
			return false;
		}

		m_pParamsWorker[i].nThreadNo = i+1;
		sprintf_s(m_pParamsWorker[i].szBuffer,sizeof(m_pParamsWorker[i].szBuffer),"%d号线程发送数据%s",i+1,m_strMessage.c_str());
		Sleep(10);

		// 如果连接服务器成功，就开始建立工作者线程，向服务器发送指定数据
		DWORD nThreadID;
		m_pParamsWorker[i].pClient = this;
		m_phWorkerThreads[i] = CreateThread(0,0,_WorkerThread,(void*)(&m_pParamsWorker[i]),0,&nThreadID);
	}

	return true;
}

// 向服务器进行socket连接
bool CClient::ConnectToServer(SOCKET* pSocket,string strServer,int nPort)
{
	//printf_s("连接服务器地址：%s:%d.\n",strServer.c_str(),nPort);
	// 生成socket
	*pSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (INVALID_SOCKET == *pSocket)
	{
		printf_s("错误：初始化socket失败，错误码：%d.\n",WSAGetLastError());
		return false;
	}

	// 生成地址信息
	hostent* pServer;
	pServer = gethostbyname(strServer.c_str()); // 根据域名转换成ip地址
	if (pServer == NULL)
	{
		closesocket(*pSocket);
		printf_s("错误：无效的服务器地址.\n");
		return false;
	}

	sockaddr_in serverAddress;
	//memset(&serverAddress,0,sizeof(serverAddress));
	ZeroMemory((char*)&serverAddress,sizeof(sockaddr_in));
	serverAddress.sin_family = AF_INET;
	//CopyMemory((char*)&serverAddress.sin_addr.s_addr,(char*)pServer->h_addr,pServer->h_length);
	serverAddress.sin_addr.s_addr = inet_addr(pServer->h_name);
	serverAddress.sin_port = ntohs(nPort);
	//serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	//serverAddress.sin_port = ntohs(8889);

	// 开始连接服务器
	if (SOCKET_ERROR == connect(*pSocket,(sockaddr*)&serverAddress,sizeof(serverAddress)))
	{
		closesocket(*pSocket);
		printf_s("错误：连接至服务器失败.\n");
		return false;
	}

	return true;
}

// 初始化WinSock2.2
bool CClient::LoadSocketLib()
{
	WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(2,2),&wsaData);
	if (NO_ERROR != nResult)
	{
		printf_s("初始化WinSock2.2失败.\n");
		return false;
	}
	return true;
}

// 开始监听
bool CClient::Start()
{
	// 建立系统退出事件通知
	m_hShutdownEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	// 启动连接线程
	DWORD nThreadID;
	THREADPARAMS_CONNECTION* pThreadParams = new THREADPARAMS_CONNECTION;
	pThreadParams->pClient = this;
	m_hConnectionThread = CreateThread(0,0,_ConnectionThread,(void*)pThreadParams,0,&nThreadID);

	return true;
}

// 停止监听
void CClient::Stop()
{
	if (m_hShutdownEvent == NULL) return;

	SetEvent(m_hShutdownEvent); // 触发关闭事件

	WaitForSingleObject(m_hConnectionThread,INFINITE); // 等待Connection线程退出

	// 关闭所有的socket
	for (int i = 0; i < m_nThreads; ++i)
	{
		closesocket(m_pParamsWorker[i].sock);
	}

	// 等待所有工作者线程退出
	WaitForMultipleObjects(m_nThreads,m_phWorkerThreads,TRUE,INFINITE);

	CleanUp(); // 清空资源

	printf_s("测试停止.\n");
}

// 清空资源
void CClient::CleanUp()
{
	if (m_hShutdownEvent == NULL) return;

	RELEASE_ARR(m_phWorkerThreads);

	RELEASE_HANDLE(m_hConnectionThread);

	RELEASE_ARR(m_pParamsWorker);

	RELEASE_HANDLE(m_hShutdownEvent);
}

// 获得本机IP地址
string CClient::GetLocalIP()
{
	char hostname[MAX_PATH];
	gethostname(hostname,MAX_PATH); // 获得本机主机名
	hostent FAR* lpHostEnt = gethostbyname(hostname);
	if (lpHostEnt == NULL)
	{
		return DEFAULT_IP;
	}
	LPSTR lpAddr = lpHostEnt->h_addr_list[0]; // 取得IP地址列表中第一个IP

	in_addr inAddr;
	memmove(&inAddr,lpAddr,4);

	m_strLocalIP = inet_ntoa(inAddr); // 转换为标准的IP地址形式

	return m_strLocalIP;
}
