#pragma once

#include <string>
#include <vector>
#include "pch.h"
#include "framework.h"
#include <list>
#include <map>
#include <mutex>

#define WM_SEND_PACK (WM_USER + 1)	//���Ͱ�����
#define WM_SEND_PACKET_ACK (WM_USER + 2)	//�������ݰ�

#pragma pack(push)
#pragma pack(1)

class CPacket
{
public: 
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) {}

	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}

		sSum = 0;
		for (int j = 0; j < nSize; j++) {
			sSum += BYTE(pData[j]) & 0xFF;
		}
	}

	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}

	//nSize���������ݴ�С�������ǻ�ȡ��������
	CPacket(const BYTE* pData, size_t& nSize) {
		size_t i = 0;
		//����pData���ҵ���0xFEFF��ͷ�İ�ͷ��
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		//���Ե���nSize��ֻ�п�������ް����ݡ�
		if (i + 4 + 2 + 2 > nSize) {	//�����ݿ��ܲ�ȫ�����߰�ͷδ��ȫ�����յ���
			nSize = 0;
			return;
		}

		nLength = *(DWORD*)(pData + i);
		i += 4;
		if (i + nLength > nSize) {		//����û�жԴ����ݰ����� ���Ͷ� �ְ�����nLength��¼���������ĳ��ȡ�
			nSize = 0;
			return;
		}

		sCmd = *(WORD*)(pData + i);
		i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			//TRACE("[�ͻ���]%s\r\n", strData.c_str() + 12);
			i += nLength - 4;
		}

		sSum = *(WORD*)(pData + i);
		i += 2;

		WORD temp = 0;
		for (int j = 0; j < strData.size(); j++) {
			temp += BYTE(strData[j]) & 0xFF;
		}

		if (temp == sSum) {
			//nSize = nLength + 2 + 4;
			nSize = i;						//��ͷǰ���������Ч���ݣ��ü�¼ȫ������������
			return;
		}
		nSize = 0;
	}

	~CPacket() {}

	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	int Size() {	//���ݵĴ�С
		return nLength + 6;
	}

	const char* Data(std::string& strOut) const {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;	pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd;	pData += 2;
		memcpy(pData, strData.c_str(), strData.size());	pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	} 

public:
	WORD sHead;
	DWORD nLength;
	WORD sCmd;
	std::string strData;
	WORD sSum;
};

#pragma pack(pop)

typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;         //�Ƿ���Ч
	BOOL IsDirectory;       //�Ƿ�ΪĿ¼ 0�� 1�� -1��Ч��Ĭ�ϣ�
	BOOL HasNext;           //�Ƿ��к��� 0û�� 1�У�Ĭ�ϣ�
	char szFileName[256];   //�ļ��� 
}FILEINFO, * PFILEINFO;


typedef struct MouseEvent {
	MouseEvent() {
		nAction = -1;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;	//���0��˫��1������2���ſ�3���޲���4�������ϻ�5�������»�6
	WORD nButton;	//���0���Ҽ�1���м�2
	POINT ptXY;		//����
}MOUSEEV, * PMOUSEEV;

enum {
	CSM_AUTOCLOSE = 1,	//CSM = client socket mode �Զ��ر�ģʽ��
};

typedef struct PacketData {		//������������������
	std::string strData;		//����
	UINT nMode;					//���ݳ���
	WPARAM AttParam;			//���Ӳ���������˵���ļ�����Ϣ����λ�ã��������ص��ļ�FILE
	PacketData(const char* pData, size_t nLen, UINT mode, WPARAM nAttParam = 0)
	{
		strData.resize(nLen);
		memcpy((char*)strData.c_str(), pData, nLen);
		nMode = mode;
		AttParam = nAttParam;
	}

	PacketData(const PacketData& data)
	{
		strData = data.strData;
		nMode = data.nMode;
		AttParam = data.AttParam;
	}

	PacketData& operator=(const PacketData& data)
	{
		if (this != &data) {
			strData = data.strData;
			nMode = data.nMode;
			AttParam = data.AttParam;
		}
		return *this;
	}
}PACKET_DATA;

//��ֹ������á�
std::string GetErrInfo(int wsaErrCode);

class CClientSocket
{
public:
	static CClientSocket* getInstance() {
		if (m_instance == NULL) {
			m_instance = new CClientSocket();
		}
		return m_instance;
	}

	bool InitSocket();

#define BUFFER_SIZE 4096000

	int DealCommand() {
		if (cli_sock == -1) return -1;
		char* buffer = m_buffer.data();

		static size_t index = 0;
		while (true) {
			size_t len = recv(cli_sock, buffer + index, BUFFER_SIZE - index, 0);
			TRACE("[�ͻ���]index = %d len = %d buffer_size = %d\n", index, len, index + len);

			if ((int)len <= 0 && (int)index == 0) {
				return -1;
			}
			index += len;
			len = index;

			//Dump((BYTE*)buffer, len);
			m_packet = CPacket((BYTE*)buffer, len);

			if (len > 0) {
				memmove(buffer, buffer + len, index - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}
	
	bool SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClose = true, WPARAM wParam = 0);	//���ڣ�������Ϣѭ�����ƣ��Լ�����ӿڡ�

	void SendPack(UINT message, WPARAM wParam, LPARAM lParam);	//���ڷ��Ͱ���Ϣ��Ӧ����

	bool GetFilePath(std::string& strPath) {
		if ((m_packet.sCmd == 2) || (m_packet.sCmd == 3) || (m_packet.sCmd == 4))
		{
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}

	bool GetMouseEvent(MOUSEEV& mouse) {
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}

	CPacket& GetPacket() {
		return m_packet;
	}

	void CloseSocket() {
		closesocket(cli_sock);
		cli_sock = INVALID_SOCKET;
	}
	
	void UpdataAddress(int nIP, int nPort) {
		m_nIP = nIP;
		m_nPort = nPort;
	}

private:
	HANDLE m_eventInvoke;	// �����¼�

	typedef void(CClientSocket::* MSGFUNC)(UINT message, WPARAM wParam, LPARAM lParam);
	std::map<UINT, MSGFUNC> m_mapFunc;

	std::mutex m_mutex;
	std::list<CPacket> m_listSendPacket;	//Ҫ���͵����ݰ�
	std::map<HANDLE, std::list<CPacket>&> m_mapRecvPacket;	// �¼�������Լ����յ������ݰ���
	std::map<HANDLE, bool> m_mapAutoClosed;					//�������ӱ��
	std::vector<char> m_buffer;

	int m_nIP;
	int m_nPort;

	UINT m_nThreadID;
	HANDLE m_hThread;
	

	//��������
	static CClientSocket* m_instance;
	SOCKET cli_sock;
	CPacket m_packet;
	
private:
	static unsigned threadEntry(void* arg);
	void threadFunc();

	bool Send(const char* pData, int nSize) {
		if (cli_sock == -1) return false;
		return send(cli_sock, pData, nSize, 0) > 0;
	}

	bool Send(const CPacket& pack) {
		if (cli_sock == -1) return false;
		std::string strOut;
		pack.Data(strOut);
		return send(cli_sock, strOut.c_str(), strOut.size(), 0) > 0;
	}

	CClientSocket& operator=(const CClientSocket& ss) {}

	//��������
	CClientSocket(const CClientSocket& ss) {
		cli_sock = ss.cli_sock;
		m_nIP = ss.m_nIP;
		m_nPort = ss.m_nPort;

		for (auto it = m_mapFunc.begin(); it != ss.m_mapFunc.end(); it++) {
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(it->first, it->second));
		}
	}

	CClientSocket() :m_nIP(INADDR_ANY), m_nPort(0), cli_sock(INVALID_SOCKET), m_hThread(INVALID_HANDLE_VALUE) {
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���"), _T("��ʼ������"), MB_OK | MB_ICONERROR);	// MB_ICONERROR��Ϣ���л����һ��ֹͣ��־ͼ�ꡣ
			exit(0);
		}
		
		m_eventInvoke = CreateEvent(NULL, TRUE, FALSE, 0);		// ����У�飬�̳߳ɹ����������¼���Ӧ��
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CClientSocket::threadEntry, this, 0, &m_nThreadID);
		if (WaitForSingleObject(m_eventInvoke, 100) == WAIT_TIMEOUT) {
			TRACE("������Ϣ�����߳�����ʧ���ˣ�\r\n");
		}
		TRACE("CClientSocket Thread Start\r\n");
		CloseHandle(m_eventInvoke);

		m_buffer.resize(BUFFER_SIZE);
		memset(m_buffer.data(), 0, BUFFER_SIZE);

		struct {
			UINT message;
			MSGFUNC func;
		}func[] = {
			{WM_SEND_PACK, &CClientSocket::SendPack},
			{0, NULL}
		};

		for (int i = 0; func[i].message != 0; i++) {
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(func[i].message, func[i].func));
		}
	}

	~CClientSocket() {
		WSACleanup();
	}

	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}

	static void releaseInstance() {
		if (m_instance != NULL) {
			CClientSocket* temp = m_instance;
			m_instance = nullptr;
			delete temp;
		}
	}

	class CHelper 
	{
	public:
		CHelper() {
			CClientSocket::getInstance();
		}

		~CHelper() {
			CClientSocket::releaseInstance();
		}
	};

	static CHelper m_helper;
};