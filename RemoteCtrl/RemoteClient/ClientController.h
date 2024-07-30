#pragma once

#include "ClientSocket.h"

#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include "WatchDlg.h"
#include <map>
#include "resource.h"
#include "Tool.h"

#define WM_SEND_PACK (WM_USER + 1)	//���Ͱ�����
#define WM_SEND_DATA (WM_USER+2)	//��������
#define WM_SEND_STATUS (WM_USER+3)	//��ʾ״̬
#define WM_SEND_WATCH (WM_USER+4)	//Զ�̼��
#define WM_SEND_MESSAGE (WM_USER+0x1000) //�Զ�����Ϣ����

class CClientController
{
public:
	static CClientController* getInstance();

	//��ʼ������
	int InitController();

	//����
	int Invoke(CWnd*& pMainWnd);

	LRESULT SendMessage(MSG msg);


	void UpdataAddress(int nIP, int nPort) {
		CClientSocket::getInstance()->UpdataAddress(nIP, nPort);
	}

	int DealCommand() {
		return CClientSocket::getInstance()->DealCommand();
	}

	void CloseSocket() {
		CClientSocket::getInstance()->CloseSocket();
	}

	bool SendPacket(const CPacket& pack) {
		CClientSocket* pClient = CClientSocket::getInstance();
		if (pClient->InitSokcet() == false)
			return false;
	
		return pClient->Send(pack);
	}

	// 1 �鿴���̷���
	// 2 �鿴ָ��Ŀ¼�µ��ļ�
	// 3 ���ļ�
	// 4 �����ļ���������ļ������ͻ��ˣ�
	// 9 ɾ���ļ�
	// 5 ������
	// 6 ������Ļ����
	// 7 ����
	// 8 ����
	// 2024 ��������
	//����cmd��ʧ�ܷ���-1��
	//Ĭ�ϣ�ֻ����һ�����ݾ͹ر����ӡ�
	int SendCommandPacket(
		int nCmd,
		bool bAutoClose = true,
		BYTE* pData = NULL,
		size_t nLength = 0) {

		CClientSocket* pClient = CClientSocket::getInstance();
		if (pClient->InitSokcet() == false)
			return false;
		pClient->Send(CPacket(nCmd, pData, nLength));
		int cmd = pClient->DealCommand();
		TRACE("ack = %d\r\n", cmd);
		if (bAutoClose)
			CloseSocket();
		return cmd;
	}

	int GetImage(CImage& image) {
		CClientSocket* pClient = CClientSocket::getInstance();
		return CTool::Byte2Image(image, pClient->GetPacket().strData); 
	}

	int DownFile(CString strPath);
	
	void StartWatchScreen();

protected:
	CClientController();

	~CClientController() {
		WaitForSingleObject(m_hThread, 100);
	}

	static void __stdcall threadEntryForWatchScreen(void* arg);
	void threadWatchScreen();

	static void __stdcall threadEntryForDownloadFile(void* arg);
	void threadForDownloadFile();

	static unsigned __stdcall threadEntry(void* arg);
	void threadFunc();

	static void releaseInstance() {
		if (m_instance != nullptr) {
			CClientController* temp = m_instance;
			m_instance = nullptr;
			delete temp;
		}
	}

	LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);

private:
	typedef struct MsgInfo {
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m) {
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}
		MsgInfo operator=(const MsgInfo& m) {
			if (this != &m) {
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			return *this;
		}
	}MSGINFO;

	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lPAram);
	static std::map<UINT, MSGFUNC> m_mapFunc;
	
	CWatchDlg m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;

	HANDLE m_hThread;
	unsigned int m_nThreadID;
	HANDLE m_hThreadDown;
	HANDLE m_hThreadWatch;

	// �����ļ���Զ��·��
	CString m_strRemote;
	// �����ļ��ı��ر���·��
	CString m_strLocal;

	bool m_isClose;	// ����߳��Ƿ�ر�

	static CClientController* m_instance;

	class CHelper {
	public:
		CHelper(){}

		~CHelper() {
			CClientController::releaseInstance();
		}
	};
	static CHelper m_helper;
};

