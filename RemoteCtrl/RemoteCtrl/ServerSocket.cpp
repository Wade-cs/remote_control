#include "pch.h"
#include "ServerSocket.h"

//CServerSocket server;

//���ⶨ��

CServerSocket* CServerSocket::m_instance = NULL;

CServerSocket::CHelper CServerSocket::m_helper;			//����������������� m_instance �������������⡣

CServerSocket* cserver = CServerSocket::getInstance();