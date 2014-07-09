#pragma once

#include <string>
#include <WinSock2.h>

#define MAX_BUFFER_LEN 8196 // ����������1024*8
#define DEFAULT_PORT 12345
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_THREADS 100
#define DEFAULT_MSG "HELLO!"

using std::string;

class CClient;

// ���ڷ������ݵ��̲߳���
struct THREADPARAMS_WORKER
{
	CClient* pClient; // ��ָ��
	SOCKET sock; // ÿ���߳�ʹ�õ�socket
	int nThreadNo; // �̱߳��
	char szBuffer[MAX_BUFFER_LEN];
};

// ����socket���ӵ��̲߳���
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
	bool LoadSocketLib(); // ����socket��
	void UnloadSocketLib() { WSACleanup(); } // ж��socket��

	bool Start(); // ��ʼ����
	void Stop(); // ֹͣ����

	string GetLocalIP(); // ��ñ���IP��ַ

	void SetIP(const string& strIP) { m_strServerIP = strIP; } // ���÷�����IP
	void SetPort(const int& nPort) { m_nPort = nPort; } // �������ӷ������˿�
	void SetThreads(const int& n) { m_nThreads = n; } // ���ò����߳�����
	void SetMessage(const string& strMessage) { m_strMessage = strMessage; } // ���÷��͵���Ϣ

private:
	bool EstablishConnections(); // ��������
	bool ConnectToServer(SOCKET* pSocket,string strServer,int nPort); // ���������������
	static DWORD WINAPI _ConnectionThread(LPVOID lpParam); // ���ڽ������ӵ��߳�
	static DWORD WINAPI _WorkerThread(LPVOID lpParam); // ���ڷ�����Ϣ���߳�

	void CleanUp(); // �ͷ���Դ

private:
	string m_strServerIP; // ��������IP��ַ
	string m_strLocalIP; // ����IP��ַ
	string m_strMessage; // ���͵���Ϣ
	int m_nPort; // �������˿�
	int m_nThreads; // �����߳�����

	HANDLE* m_phWorkerThreads; // �����߳̾������
	HANDLE m_hConnectionThread; // �����̵߳ľ��
	HANDLE m_hShutdownEvent; // ϵͳֹͣ�˳��¼�

	THREADPARAMS_WORKER* m_pParamsWorker; // �̲߳���
};
