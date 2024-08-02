#include "pch.h"
#include "ClientSocket.h"
#include "Tool.h"


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


bool CClientSocket::InitSocket()
{
	if (cli_sock != INVALID_SOCKET) {
		CloseSocket();
	}
	cli_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (cli_sock == -1) return false;
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(m_nIP);
	server_addr.sin_port = htons(m_nPort);

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


bool CClientSocket::SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed, WPARAM AttParam)
{
	// TODO��������Ϣ���͹���Ƶ������ʹ�ر�����صĴ��ڻ���ֹͣ���͡���Ϣ�����ﻹ��ʣ�����Ϣ��
	UINT nMode = isAutoClosed ? CSM_AUTOCLOSE : 0;

	std::string strOut;
	pack.Data(strOut);

	PACKET_DATA* pData = new PACKET_DATA(strOut.c_str(), strOut.size(), nMode, AttParam);
	bool ret = PostThreadMessage(m_nThreadID, WM_SEND_PACK, (WPARAM)pData, (LPARAM)hWnd);
	if (ret == false) {
		TRACE("PostThreadMessage error : %d \r\n", ::GetLastError());
		delete pData;
	}
	return ret;
}

//������������Ϣ����
void CClientSocket::SendPack(UINT message, WPARAM wParam, LPARAM lParam)
{
	PACKET_DATA data = *(PACKET_DATA*)wParam;
	delete (PACKET_DATA*)wParam;

	// TODO:��ش��ڹرգ�hWndû�ˡ���Ϣ�����ﻹ����ص����������ǰ�ж�hWnd��Ӧ�Ĵ����Ƿ��ڡ�
	HWND hWnd = (HWND)lParam;
	if (IsWindow(hWnd) == 0)
	{
		return;
	}

	if (InitSocket() == true) {

		int ret = send(cli_sock, (char*)data.strData.c_str(), (int)data.strData.size(), 0);

		if (ret > 0) {
			size_t index = 0;
			std::string strBufer;
			strBufer.resize(BUFFER_SIZE);
			char* pBuffer = (char*)strBufer.c_str();

			while (cli_sock != INVALID_SOCKET) {
				int len = recv(cli_sock, pBuffer + index, BUFFER_SIZE - index, 0);
				TRACE("index = %d , len = %d \r\n", index, len);
				if ((len > 0) || (index > 0)) {
					index += (size_t)len;
					size_t nLen = index;
					CPacket pack((BYTE*)pBuffer, nLen);
					if (nLen > 0) {
						if (IsWindow(hWnd) != 0)
						{
							CPacket* temp = new CPacket(pack);
 							LRESULT ret = ::SendMessage(hWnd, WM_SEND_PACKET_ACK, (WPARAM)temp, data.AttParam);
							//bool ret = ::PostMessage(hWnd, WM_SEND_PACKET_ACK, (WPARAM)temp, data.AttParam);	//�������ڶ�������ڹرգ�����ȡ�����������pack�ѿռ�û���ͷš�
							//TRACE("ret = %d\r\n", ret);
							if (ret	== 0) {
								//���͵ĵ�ʱ�����ڸպùرգ�����ʧ�ܡ�
								delete temp;
							}
						}


						if (data.nMode & CSM_AUTOCLOSE) {
							CloseSocket();
							return;
						}
						index -= nLen;
						memmove(pBuffer, pBuffer + nLen, index);
					}
				}
				else {
					CloseSocket();
					::SendMessage(hWnd, WM_SEND_PACKET_ACK, NULL, 1);
				}
			}
		}
		else {
			CloseSocket();
			::SendMessage(hWnd, WM_SEND_PACKET_ACK, NULL, -1);
		}
	}
	else {
		::SendMessage(hWnd, WM_SEND_PACKET_ACK, NULL, -2);
	}
}

unsigned CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc();
	ExitThread(0);
	return 0;
}

void CClientSocket::threadFunc()
{
	SetEvent(m_eventInvoke);

	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (m_mapFunc.find(msg.message) != m_mapFunc.end()) {
			(this->*m_mapFunc[msg.message])(msg.message, msg.wParam, msg.lParam);
		}
	}

}
