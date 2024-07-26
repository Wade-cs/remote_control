#pragma once

#include "pch.h"
#include "framework.h"
#include <list>
#include "Packet.h"

typedef void(*SOCKET_CALLBACK)(void* arg, int status , std::list<CPacket>& listPacket, CPacket& inPacket);

class CServerSocket
{
public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL) {
			m_instance = new CServerSocket();
		}
		return m_instance;
	}

	bool InitSocket(short port) {
		if (serv_sock == -1)	return false;
		sockaddr_in serv_addr;
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(port);
		serv_addr.sin_addr.s_addr = INADDR_ANY;

		if (bind(serv_sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
			printf("%d\r\n", GetLastError());
			return false;
		}

		if (listen(serv_sock, 1) == -1) {
			return false;
		}

		return true;
	}

	int Run(SOCKET_CALLBACK callback, void* arg, short port = 9527) {
		// 1 ���ȵĿɿ��� 2 �Խӵķ����� 3 ���������������籩¶����
		// TOD0: socket��bind��listen��accept��read��write��close//�׽��ֳ�ʼ��
		
		//�׽��ֳ�ʼ��
		if (InitSocket(port) == false) { 
			return -1;
		}

		std::list<CPacket> listPacket;
		m_callback = callback;
		m_arg = arg;

		int count = 0;
		while (getInstance() != NULL) {
			if (AcceptClient() == false) {
				if (count >= 3) {
					return -2;
				}
				count++;
			}
			int ret = DealCommand();

			if (ret > 0) {
				m_callback(m_arg, ret, listPacket, m_packet);
				while (listPacket.size() > 0) {
					Send(listPacket.front());
					listPacket.pop_front();
				}
			}

			CloseClient();
		}

		return 0;
	}

	bool AcceptClient() {
		sockaddr_in cli_addr;
		int cli_sz = sizeof(cli_addr);
		cli_sock = accept(serv_sock, (sockaddr*)&cli_addr, &cli_sz);
		if (cli_sock == -1)	return false;
		return true;
	}

#define BUFFER_SIZE 4096

	//��ȡ����
	int DealCommand() {
		if (cli_sock == -1) return -1;
		//char buffer[1024] = "";
		char* buffer = new char[BUFFER_SIZE];	//�涨�ã�����˽��տͻ��˵����ݣ������ӡ�ÿ��ֻ����һ��ָ��
		memset(buffer, 0, sizeof(buffer));
		size_t index = 0;		
		while (true) {
			size_t len = recv(cli_sock, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0) {
				delete[]buffer;
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);

			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;				//index �������Ϊ��ǰһ�ν����δ����ʣ���ֽڵĸ�λ��
				delete[]buffer;
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

	CPacket& GetPacket()  {
		return m_packet;
	}

	void CloseClient() {
		closesocket(cli_sock);
		cli_sock = INVALID_SOCKET;
	}

private:
	SOCKET serv_sock;
	SOCKET cli_sock;
	CPacket m_packet;

	SOCKET_CALLBACK m_callback;
	void* m_arg;

	CServerSocket& operator=(const CServerSocket& ss) {}

	//��������
	CServerSocket(const CServerSocket& ss) {
		serv_sock = ss.serv_sock;
		cli_sock = ss.cli_sock;
	}

	//����
	CServerSocket() {
		if (InitSockEnv() == FALSE){
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���"), _T("��ʼ������"), MB_OK | MB_ICONERROR);	// MB_ICONERROR��Ϣ���л����һ��ֹͣ��־ͼ�ꡣ
			exit(0);
		}
		serv_sock = socket(PF_INET, SOCK_STREAM, 0);
		cli_sock = INVALID_SOCKET;
	};

	~CServerSocket() {
		closesocket(serv_sock);
		WSACleanup();
	};

	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}
	
	static void releaseInstance() {
		if (m_instance != NULL) {
			CServerSocket* temp = m_instance;
			m_instance = nullptr;
			delete temp;
		}
	}

	//��������
	static CServerSocket* m_instance;

	class CHelper
	{
	public:
		CHelper() {
			CServerSocket::getInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};

	static CHelper m_helper;		// ��̬��Ա����:
};