#pragma once

#include <string>
#include <vector>
#include "pch.h"
#include "framework.h"

#pragma pack(push)
#pragma pack(1)

void Dump(BYTE* pData, size_t nSize);


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

	const char* Data() {
		strOut.resize(Size());
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
	std::string strOut;
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

	//���޴ε���
	//bool InitSokcet(const std::string& serverIPAddrness) {
	bool InitSokcet(int nIP, int nPort) {
		if (cli_sock != INVALID_SOCKET) {
			CloseSocket();
		}
		cli_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (cli_sock == -1) return false;
		sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;
		//server_addr.sin_addr.s_addr = inet_addr(serverIPAddrness.c_str());
		//server_addr.sin_port = htons(9527);
		
		//server_addr.sin_addr.s_addr = nIP;			//�ֽ������⣡
		server_addr.sin_addr.s_addr = htonl(nIP);	
		server_addr.sin_port = htons(nPort);
		
		if (server_addr.sin_addr.s_addr == INADDR_NONE) {
			AfxMessageBox(_T("ָ����Ip��ַ�����ڣ�"));
			return false;
		}

		int ret = connect(cli_sock, (sockaddr*)&server_addr, sizeof(sockaddr));
		
		if (ret == -1) {
			AfxMessageBox(_T("conncet����ʧ�ܣ�"));
			TRACE("[�ͻ���]����ʧ�ܣ�%d %s\r\n", WSAGetLastError(), GetErrInfo(WSAGetLastError()).c_str());
			return false;
		}
		return true;
	}

#define BUFFER_SIZE 40960000

	int DealCommand() {
		if (cli_sock == -1) return -1;
		//�ͻ��˻��յ�����˶�����ݰ�
		//char* buffer = new char[BUFFER_SIZE];
		char* buffer = m_buffer.data();

		//������һ���ļ�����֮��indexһ����ص�0����������ٴε���ļ�����������ô������һ�ε����������bug
		//ĳ��������д���˵Ļ���bug���Ұ��졣
		static size_t index = 0;		
		while (true) {
			size_t len = recv(cli_sock, buffer + index, BUFFER_SIZE - index, 0);
			TRACE("[�ͻ���]index = %d len = %d buffer_size = %d\n", index, len, index + len);

			// 1. ֮ǰbuffer��index�ֲ����㣬�����ԵĶ����ݡ�
			// 2. �ر��ǣ�index��Ϊ0������£���������󼸸���һ����յ��ˣ��ٺ󼸴ε�recv��Ϊ0��index�������ݣ���
			if (len <= 0 && index == 0) {
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

	bool Send(const char* pData, int nSize) {
		if (cli_sock == -1) return false;
		return send(cli_sock, pData, nSize, 0) > 0;
	}

	bool Send(CPacket& pack) {
		if (cli_sock == -1) return false;
		return send(cli_sock, pack.Data(), pack.Size(), 0) > 0;
	}

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

private:
	std::vector<char> m_buffer;

	//��������
	static CClientSocket* m_instance;
	SOCKET cli_sock;
	CPacket m_packet;
	

	CClientSocket& operator=(const CClientSocket& ss) {}

	//��������
	CClientSocket(const CClientSocket& ss) {
		cli_sock = ss.cli_sock;
	}

	CClientSocket() {
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���"), _T("��ʼ������"), MB_OK | MB_ICONERROR);	// MB_ICONERROR��Ϣ���л����һ��ֹͣ��־ͼ�ꡣ
			exit(0);
		}
		//�ͻ�����Ҫ��InitSock�����ʼ��cli_sock
		//cli_sock = socket(PF_INET, SOCK_STREAM, 0);
		m_buffer.resize(BUFFER_SIZE);
		memset(m_buffer.data(), 0, BUFFER_SIZE);
	}

	~CClientSocket() {
		//closesocket(cli_sock);
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

