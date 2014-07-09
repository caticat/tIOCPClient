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

// �������ӵ��߳�
DWORD WINAPI CClient::_ConnectionThread(LPVOID lpParam)
{
	THREADPARAMS_CONNECTION* pParams = (THREADPARAMS_CONNECTION*)lpParam;
	CClient* pClient = pParams->pClient;

	printf_s("_AcceptThread����,ϵͳ������.\n");
	pClient->EstablishConnections();
	printf_s("_ConnectionThread�߳̽���.\n");

	RELEASE(pParams);
	return 0;
}

// ���ڷ�����Ϣ���߳�
DWORD WINAPI CClient::_WorkerThread(LPVOID lpParam)
{
	THREADPARAMS_WORKER* pParams = (THREADPARAMS_WORKER*)lpParam;
	CClient* pClient = pParams->pClient;

	char szTemp[MAX_BUFFER_LEN] = {0};
	char szRecv[MAX_BUFFER_LEN] = {0};

	int nByteSent = 0;
	int nByteRecv = 0;

	// ����������͵�һ����Ϣ
	sprintf_s(szTemp,sizeof(szTemp),"��1����Ϣ��%s",pParams->szBuffer);
	nByteSent = send(pParams->sock,szTemp,strlen(szTemp),0);
	if (SOCKET_ERROR == nByteSent)
	{
		printf_s("���󣺵�1�η�����Ϣʧ�ܣ������룺%d.\n",WSAGetLastError());
		return 1;
	}
	printf_s("�������������Ϣ�ɹ���%s.\n",szTemp);
	Sleep(3000);

	// ����������͵ڶ�����Ϣ
	memset(szTemp,0,sizeof(szTemp));
	sprintf_s(szTemp,sizeof(szTemp),"��2����Ϣ��%s",pParams->szBuffer);
	nByteSent = send(pParams->sock,szTemp,strlen(szTemp),0);
	if (SOCKET_ERROR == nByteSent)
	{
		printf_s("���󣺵�2�η�����Ϣʧ�ܣ������룺%d.\n",WSAGetLastError());
		return 1;
	}
	printf_s("�������������Ϣ�ɹ���%s.\n",szTemp);
	Sleep(3000);

	// ����������͵�������Ϣ
	memset(szTemp,0,sizeof(szTemp));
	sprintf_s(szTemp,sizeof(szTemp),"��3����Ϣ��%s",pParams->szBuffer);
	nByteSent = send(pParams->sock,szTemp,strlen(szTemp),0);
	if (SOCKET_ERROR == nByteSent)
	{
		printf_s("���󣺵�3�η�����Ϣʧ�ܣ������룺%d.\n",WSAGetLastError());
		return 1;
	}
	printf_s("�������������Ϣ�ɹ���%s.\n",szTemp);

	if (pParams->nThreadNo == pClient->m_nThreads) // ����о�����
	{
		printf_s("���Բ���%d���߳����.\n",pClient->m_nThreads);
	}
	return 0;
}

// ��������
bool CClient::EstablishConnections()
{
	m_phWorkerThreads = new HANDLE[m_nThreads];
	m_pParamsWorker = new THREADPARAMS_WORKER[m_nThreads];

	for (int i = 0; i < m_nThreads; ++i)
	{
		// �����û�ֹͣ�¼�
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_hShutdownEvent,0))
		{
			printf_s("���յ��û�ֹͣ����.\n");
			return true;
		}

		// ���ӷ�����
		if (!this->ConnectToServer(&m_pParamsWorker[i].sock,m_strServerIP,m_nPort))
		{
			printf_s("���ӷ�����ʧ��.\n");
			return false;
		}

		m_pParamsWorker[i].nThreadNo = i+1;
		sprintf_s(m_pParamsWorker[i].szBuffer,sizeof(m_pParamsWorker[i].szBuffer),"%d���̷߳�������%s",i+1,m_strMessage.c_str());
		Sleep(10);

		// ������ӷ������ɹ����Ϳ�ʼ�����������̣߳������������ָ������
		DWORD nThreadID;
		m_pParamsWorker[i].pClient = this;
		m_phWorkerThreads[i] = CreateThread(0,0,_WorkerThread,(void*)(&m_pParamsWorker[i]),0,&nThreadID);
	}

	return true;
}

// �����������socket����
bool CClient::ConnectToServer(SOCKET* pSocket,string strServer,int nPort)
{
	//printf_s("���ӷ�������ַ��%s:%d.\n",strServer.c_str(),nPort);
	// ����socket
	*pSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (INVALID_SOCKET == *pSocket)
	{
		printf_s("���󣺳�ʼ��socketʧ�ܣ������룺%d.\n",WSAGetLastError());
		return false;
	}

	// ���ɵ�ַ��Ϣ
	hostent* pServer;
	pServer = gethostbyname(strServer.c_str()); // ��������ת����ip��ַ
	if (pServer == NULL)
	{
		closesocket(*pSocket);
		printf_s("������Ч�ķ�������ַ.\n");
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

	// ��ʼ���ӷ�����
	if (SOCKET_ERROR == connect(*pSocket,(sockaddr*)&serverAddress,sizeof(serverAddress)))
	{
		closesocket(*pSocket);
		printf_s("����������������ʧ��.\n");
		return false;
	}

	return true;
}

// ��ʼ��WinSock2.2
bool CClient::LoadSocketLib()
{
	WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(2,2),&wsaData);
	if (NO_ERROR != nResult)
	{
		printf_s("��ʼ��WinSock2.2ʧ��.\n");
		return false;
	}
	return true;
}

// ��ʼ����
bool CClient::Start()
{
	// ����ϵͳ�˳��¼�֪ͨ
	m_hShutdownEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	// ���������߳�
	DWORD nThreadID;
	THREADPARAMS_CONNECTION* pThreadParams = new THREADPARAMS_CONNECTION;
	pThreadParams->pClient = this;
	m_hConnectionThread = CreateThread(0,0,_ConnectionThread,(void*)pThreadParams,0,&nThreadID);

	return true;
}

// ֹͣ����
void CClient::Stop()
{
	if (m_hShutdownEvent == NULL) return;

	SetEvent(m_hShutdownEvent); // �����ر��¼�

	WaitForSingleObject(m_hConnectionThread,INFINITE); // �ȴ�Connection�߳��˳�

	// �ر����е�socket
	for (int i = 0; i < m_nThreads; ++i)
	{
		closesocket(m_pParamsWorker[i].sock);
	}

	// �ȴ����й������߳��˳�
	WaitForMultipleObjects(m_nThreads,m_phWorkerThreads,TRUE,INFINITE);

	CleanUp(); // �����Դ

	printf_s("����ֹͣ.\n");
}

// �����Դ
void CClient::CleanUp()
{
	if (m_hShutdownEvent == NULL) return;

	RELEASE_ARR(m_phWorkerThreads);

	RELEASE_HANDLE(m_hConnectionThread);

	RELEASE_ARR(m_pParamsWorker);

	RELEASE_HANDLE(m_hShutdownEvent);
}

// ��ñ���IP��ַ
string CClient::GetLocalIP()
{
	char hostname[MAX_PATH];
	gethostname(hostname,MAX_PATH); // ��ñ���������
	hostent FAR* lpHostEnt = gethostbyname(hostname);
	if (lpHostEnt == NULL)
	{
		return DEFAULT_IP;
	}
	LPSTR lpAddr = lpHostEnt->h_addr_list[0]; // ȡ��IP��ַ�б��е�һ��IP

	in_addr inAddr;
	memmove(&inAddr,lpAddr,4);

	m_strLocalIP = inet_ntoa(inAddr); // ת��Ϊ��׼��IP��ַ��ʽ

	return m_strLocalIP;
}
