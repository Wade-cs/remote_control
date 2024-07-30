#include "pch.h"
#include "ClientSocket.h"


CClientSocket* CClientSocket::m_instance = NULL;

CClientSocket::CHelper CClientSocket::m_helper;

CClientSocket* cclient = CClientSocket::getInstance();


std::string GetErrInfo(int wsaErrCode)
{
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}


void Dump(BYTE* pData, size_t nSize) {
	std::string strOut;
	for (size_t i = 0; i < nSize; i++) {
		char buf[8] = "";
		if (i % 15 == 0) strOut += "\n";
		snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
		strOut += buf;
	}
	strOut += "\n";
	OutputDebugStringA(strOut.c_str());
}

void CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc();
	ExitThread(0);
}

void CClientSocket::threadFunc()
{
	if (InitSocket() == false) {
		return;
	}
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();

	int index = 0;
	while (cli_sock != INVALID_SOCKET)
	{
		if (m_listSendPacket.size() > 0) 
		{
			CPacket& head = m_listSendPacket.front();
			if (Send(head) == false)
			{
				TRACE("����ʧ��\r\n");
				continue;
			}
			auto it = m_mapRecvPacket.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list<CPacket>()));

			int length = recv(cli_sock, pBuffer + index, BUFFER_SIZE - index, 0);

			if (length > 0 || index > 0) {
				index += length;
				size_t size = (size_t)index;
				CPacket pack((BYTE*)pBuffer, size);
				if (size > 0) {
					pack.hEvent = head.hEvent;
					it.first->second.push_back(pack);
					SetEvent(head.hEvent);
				}
			}
			else if (length <= 0 && index <= 0) 
			{
				CloseSocket();
			}
			//TODO	����������� ���ڻ�ȡ�ļ�����Ϣ�����ֻ���ڵ�һ��������hEvent��ɾ����֮������޷�֪ͨ�ˡ�
			m_listSendPacket.pop_front();
		}
	}
}
