#include "pch.h"
#include "ClientController.h"

std::map<UINT, CClientController::MSGFUNC> CClientController::m_mapFunc;

CClientController* CClientController::m_instance = NULL;

CClientController::CHelper CClientController::m_helper;

int CClientController::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t nLength, std::list<CPacket>* recvPackets)
{
	CClientSocket* pClient = CClientSocket::getInstance();

	HANDLE hEvnet = CreateEvent(NULL, TRUE, FALSE, NULL);

	std::list<CPacket> lstPackets;
	if (recvPackets == NULL) {
		recvPackets = &lstPackets;
	}

	pClient->SendPacket(CPacket(nCmd, pData, nLength, hEvnet), *recvPackets, bAutoClose);

	CloseHandle(hEvnet);
	if (recvPackets->size() > 0) {
		return recvPackets->front().sCmd;
	}

	return -1;
}

int CClientController::DownFile(CString strPath)
{
	CFileDialog dlg(FALSE, "*",
		strPath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		NULL, &m_remoteDlg);

	if (dlg.DoModal() == IDOK) {
		m_strRemote = strPath;
		m_strLocal = dlg.GetPathName();

		m_hThreadDown = (HANDLE)_beginthread(&CClientController::threadEntryForDownloadFile, 0, this);
		// �մ�����û��wait����ʱ�߳̿�ʼִ�С�
		if (WaitForSingleObject(m_hThreadDown, 0) != WAIT_TIMEOUT) {
			return -1;
		}
		m_remoteDlg.BeginWaitCursor();		// ���ù��Ϊ�ȴ�״̬��
		m_statusDlg.m_info.SetWindowText(_T("����ִ����..."));
		m_statusDlg.ShowWindow(SW_SHOW);
		m_statusDlg.CenterWindow(&m_remoteDlg);
		m_statusDlg.SetActiveWindow();
	}
	return 0;
}

void CClientController::StartWatchScreen()
{
	m_isClose = false;

	m_hThreadWatch = (HANDLE)_beginthread(&CClientController::threadEntryForWatchScreen, 0, this);
	m_watchDlg.DoModal();

	m_isClose = true;
	WaitForSingleObject(m_hThreadWatch, 500);
}

CClientController::CClientController() :
	//��ʼ��ָ��������
	m_statusDlg(&m_remoteDlg),
	m_watchDlg(&m_remoteDlg),
	m_hThread(INVALID_HANDLE_VALUE),
	m_nThreadID(-1),
	m_isClose(true)
{
	struct {
		UINT nMsg;
		MSGFUNC func;
	}data[] = {
		{WM_SEND_PACK, &CClientController::OnShowStatus},
		{WM_SEND_PACK, &CClientController::OnShowWatcher},
		{(UINT)-1, NULL},
	};

	for (int i = 0; data[i].func != NULL; i++) {
		m_mapFunc.insert(std::pair<UINT, MSGFUNC>(data[i].nMsg, data[i].func));
	}
}

CClientController* CClientController::getInstance()
{
	static CClientController instance;
	return &instance;
}

int CClientController::InitController()
{
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CClientController::threadEntry, this, 0, &m_nThreadID);
	m_statusDlg.Create(IDD_DLG_STATUS, &m_remoteDlg);
	return 0;
}

int CClientController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_remoteDlg;
	return m_remoteDlg.DoModal();
}

LRESULT CClientController::SendMessage(MSG msg)
{ 
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL)return -2;

	MSGINFO info(msg);
	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE, (WPARAM)&info, (LPARAM)&hEvent);
	//ͨ���¼�֪ͨ����ͨ���ṹ��洢�����
	WaitForSingleObject(hEvent, INFINITE);
	CloseHandle(hEvent);

	return info.result;
}


void __stdcall CClientController::threadEntryForWatchScreen(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadWatchScreen();
	_endthreadex(0);
}


void CClientController::threadWatchScreen()
{
	Sleep(50);
	while (!m_isClose) {
		if (m_watchDlg.isFull() == false) {
			std::list<CPacket> recvPackets;
			int ret = SendCommandPacket(6, true, NULL , 0, &recvPackets);
			if (ret == 6) {
				if (CTool::Byte2Image(m_watchDlg.GetImage(), recvPackets.front().strData) == 0) {
					m_watchDlg.setImageStatus(true);
				}
				else {
					TRACE("��ȡͼƬʧ�ܣ�\r\n");
				}
			}
		}
		else {
			Sleep(1);
		}
	}
}


void __stdcall CClientController::threadEntryForDownloadFile(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadForDownloadFile();
	_endthreadex(0);
}


void CClientController::threadForDownloadFile()
{
	FILE* pFile = fopen(m_strLocal, "wb+");
	if (pFile == NULL) {
		AfxMessageBox("û��Ȩ�ޱ�����ļ��������ļ��޷�����������");
		m_statusDlg.ShowWindow(SW_HIDE);
		m_remoteDlg.EndWaitCursor();
		return;
	}
	TRACE("[�ͻ���]%s\r\n", LPCSTR(m_strRemote));


	CClientController* pController = CClientController::getInstance();
	std::list<CPacket> recvCPackets;

	do {
		int ret = pController->SendCommandPacket(4, false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength(), &recvCPackets);
		if (ret < 0) {
			AfxMessageBox("ִ����������ʧ�ܣ�");
			TRACE("ִ����������ʧ�ܣ�ret = %d\r\n", ret);
			break;
		}
		long long nLength = *(long long*)recvCPackets.front().strData.c_str(); //�����ص��ļ�����
		if (nLength == 0) {
			AfxMessageBox("�ļ�����Ϊ�㣬�����޷���ȡ�ļ�������");
			break;
		}

		recvCPackets.pop_front();

		// CString --> LPCSTR --> BYTE*
		m_statusDlg.download_process.SetRange(0, 100);
		long long nCount = 0;
		while (nCount < nLength)
		{
			double cur_portion = nCount * 100.0 / nLength;
			m_statusDlg.download_process.SetPos(cur_portion);

			CString temp;
			temp.Format(_T("����ִ���� ���� = %f %%"), cur_portion);
			m_statusDlg.m_info.SetWindowText(temp);

			CPacket cur = recvCPackets.front();

			size_t cur_size = cur.strData.size();

			fwrite(cur.strData.c_str(), 1, cur_size, pFile);

			nCount += cur_size;

			recvCPackets.pop_front();
		}

		if(nCount == nLength) m_remoteDlg.MessageBox(_T("������ɣ�"), _T("���"));
	} while (0);
	fclose(pFile);
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
}



unsigned __stdcall CClientController::threadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();
	_endthreadex(0);
	return 0;
}


void CClientController::threadFunc()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_SEND_MESSAGE) {
			//Ϊ�˻�ȡ�������ķ���ֵ��������Զ�����Ϣ�Լ���Ϣ��
			MSGINFO* pmsgInfo = (MSGINFO*)msg.wParam;
			HANDLE* pEvent = (HANDLE*)msg.lParam;

			auto it = m_mapFunc.find(pmsgInfo->msg.message);
			if (it != m_mapFunc.end()) {
				pmsgInfo->result = (this->*it->second)(pmsgInfo->msg.message, pmsgInfo->msg.wParam, pmsgInfo->msg.lParam);
			}
			else {
				pmsgInfo->result = -1;
			}
			SetEvent(pEvent);
		}
		else {
			auto it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end()) {
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);
			}
		}

	}
}

LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_watchDlg.DoModal();
}
