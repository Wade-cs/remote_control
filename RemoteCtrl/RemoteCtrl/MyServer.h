#pragma once
#include "MyThread.h"
#include "pch.h"
#include <map>
#include <MSWSock.h>

enum ServerOperator{
    MNone,
    MAccept,
    MRecv,
    MSend,
    MError
};

class CMyServer;
class MyClient;
typedef std::shared_ptr<MyClient> PCLIENT;


class CBaseOverlapped
{
public:
    DWORD m_operator;       // enum ServerOperator
    OVERLAPPED m_overlapped;
    std::vector<char> m_buffer;

    ThreadWorker m_worker;  // ������
    CMyServer* m_server;    // ������
    MyClient* m_pClient;    // ��Ӧ�Ŀͻ��ˡ�֮ǰ�� PCLIENT �������Լ������Լ�����������ͷ� map ��� client �����������������
 
    WSABUF m_wsaBuffer;     //

    virtual ~CBaseOverlapped() {
        m_buffer.clear();

    }
};


template<ServerOperator op>
class AcceptOverlapped : public CBaseOverlapped, ThreadFuncBase
{
public:
    AcceptOverlapped() {
        m_operator = MAccept;
        m_worker = ThreadWorker(this, (FUNCTYPE)&AcceptOverlapped::AcceptWorker);
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        m_buffer.resize(1024);
        m_server = NULL;
    }

    int AcceptWorker() {
        // TODO:
        INT lLength = 0, rLength = 0;
        if (*(LPDWORD)*m_pClient > 0) {
            GetAcceptExSockaddrs(*m_pClient, 0,
                sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
                (sockaddr**)m_pClient->GetLocalAddr(), &lLength,      //���ص�ַ
                (sockaddr**)m_pClient->GetRemoteAddr(), &rLength       //Զ�̵�ַ
            );
            // WSARecv ֪ͨ IOCP �����Ϣ��֮����ȥ recv
            int ret = WSARecv((SOCKET)*m_pClient,  m_pClient->RecvWSABuf(), 1, *m_pClient, &m_pClient->flags(), *m_pClient, NULL);
            if (ret == SOCKET_ERROR && (WSAGetLastError() != WSA_IO_PENDING)) {
                //TODO������
                CTool::ShowError();
            }

            // ����֮���ٴ�Accept������һ�����ӡ�
            if (!m_server->NewAccept())
            {
                return -2;
            }
        }
        return -1;
    }
};
typedef AcceptOverlapped<MAccept> ACCEPTOVERLAPPED;


template<ServerOperator>
class RecvOverlapped : public CBaseOverlapped, ThreadFuncBase
{
public:
    RecvOverlapped() {
        m_operator = MRecv;
        m_worker = ThreadWorker(this, (FUNCTYPE)&RecvOverlapped::RecvWorker);
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        m_buffer.resize(1024 * 256);
    }

    int RecvWorker() {
        int ret = m_pClient->Recv();
        return ret;
    }
};
typedef RecvOverlapped<MRecv> RECVOVERLAPPED;


template<ServerOperator>
class SendOverlapped : public CBaseOverlapped, ThreadFuncBase
{
public:
    SendOverlapped() {
        m_operator = MSend;
        m_worker = ThreadWorker(this, (FUNCTYPE)&SendOverlapped::SendWorker);
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        m_buffer.resize(1024 * 256);
    }

    int SendWorker() {
        // TODO:
        /*
            1. Send���ܲ���������ɡ�

        */

        return -1;
    }
};
typedef SendOverlapped<MSend> SENDOVERLAPPED;


template<ServerOperator>
class ErrorOverlapped : public CBaseOverlapped, ThreadFuncBase
{
public:
    ErrorOverlapped(){
        m_operator = MError;
        m_worker = ThreadWorker(this, (FUNCTYPE)&ErrorOverlapped::ErrorWorker);
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        m_buffer.resize(1024 * 256);
    }

    int ErrorWorker() {
        // TODO:
        return -1;
    }
};
typedef ErrorOverlapped<MError> ERROROVERLAPPED;

class MyClient :public ThreadFuncBase{
public:
    MyClient() :
        m_isBusy(false), m_flags(0),
        m_overlapped(new ACCEPTOVERLAPPED()),
        m_recvOverlapped(new RECVOVERLAPPED()),
        m_sendOverlapped(new SENDOVERLAPPED()),
        m_vecSend(this, (SENDCALLBACK)&MyClient::SendData)      // ��SENDCALLBACK����Ϊ�Ǳ��ǹ��� T �ı�������֪�� T �Ƕ��٣����Ի��ø�һ��������
    {
        m_sock = WSASocketW(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        m_buffer.resize(1024);
        memset(&m_laddr, 0, sizeof(sockaddr_in));
        memset(&m_raddr, 0, sizeof(sockaddr_in));
    }

    ~MyClient() {
        closesocket(m_sock);
        m_overlapped.reset();
        m_recvOverlapped.reset();
        m_sendOverlapped.reset();
        m_buffer.clear();
        m_vecSend.Clear();
    }

    void SetOverlapped(PCLIENT& ptr) {
        // ʹ�� overlapped ���Է��ʶ�Ӧ�� m_pClient
        m_overlapped->m_pClient = ptr.get();
        m_recvOverlapped->m_pClient = ptr.get();
        m_sendOverlapped->m_pClient = ptr.get();
    }

    operator SOCKET() {
        return m_sock;
    }

    // ���㽫����ת��Ϊ PVOID �� void* ����ʱ�����Զ������������������ȡ�����ָ��ֵ��
    operator PVOID() {
        return &m_buffer[0];
    }

    operator LPDWORD() {
        return &m_received;
    }

    operator LPOVERLAPPED() {
        return &(m_overlapped->m_overlapped);
    }
    
    LPWSABUF RecvWSABuf() {
        return &m_recvOverlapped->m_wsaBuffer;
    }

    LPWSABUF SendWSABuf() {
        return &m_sendOverlapped->m_wsaBuffer;
    }

    sockaddr_in* GetLocalAddr() { return &m_laddr; }
    sockaddr_in* GetRemoteAddr() { return &m_raddr; }
    size_t GetBufferSize() const { return m_buffer.size(); }
    DWORD& flags() { return m_flags; }

    int Recv() {
        int ret = recv(m_sock, m_buffer.data() + m_usedIndex, m_buffer.size() - m_usedIndex, 0);
        if (ret <= 0) return -1;
        m_usedIndex += (size_t)ret;
        // TODO :��������
        return 0;
    }

    int Send(void* buffer, size_t nSize) {
        std::vector<char> data(nSize);
        memcpy(data.data(), buffer, nSize);
        if (m_vecSend.PushBack(data) == false) {
            return -1;
        }
        return 0;
    }

    int SendData(std::vector<char>& data) {
        // TODO��ʵ���ϴӶ�����ȡ���������ݡ�
        if (m_vecSend.Size() > 0) {
            int ret = WSASend(m_sock, SendWSABuf(), 1, &m_received, m_flags, &m_sendOverlapped->m_overlapped, NULL);
            if (ret != 0 && (WSAGetLastError() != WSA_IO_PENDING)) {
                CTool::ShowError();
                return -1;
            }
        }
        return 0; 
    }

private:
    SOCKET m_sock;
    DWORD m_received;
    DWORD m_flags;
    std::shared_ptr<ACCEPTOVERLAPPED> m_overlapped;  // ����������Ϊ�˵õ�����������������������Ϣ��
    std::shared_ptr<RECVOVERLAPPED> m_recvOverlapped;
    std::shared_ptr<SENDOVERLAPPED> m_sendOverlapped;
    std::vector<char> m_buffer;     // ����Accept
    size_t m_usedIndex;             // �Ѿ�ʹ���˵Ļ�����
    sockaddr_in m_laddr;
    sockaddr_in m_raddr;
    bool m_isBusy;
    
    CMySendQueue<std::vector<char>> m_vecSend;      // �������ݶ���
};

class CMyServer :
    public ThreadFuncBase
{
public:
    CMyServer(const std::string& IP = "0.0.0.0" , short Port = 9527): threadPool(10){
        m_hIocp = INVALID_HANDLE_VALUE;
        m_sock = INVALID_SOCKET;

        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(Port);
        m_addr.sin_addr.s_addr = inet_addr(IP.c_str());
    }

    ~CMyServer(){
        closesocket(m_sock);
        for (auto it = m_clients.begin(); it != m_clients.end(); it++) {
            it->second.reset();     // share_ptr �����Լ��������Լ���������Բ���������
        }
        m_clients.clear();
        CloseHandle(m_hIocp);
        threadPool.Stop();
    }

    bool StartService() {
        m_sock = WSASocketW(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        int opt = -1;
        setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

        if (bind(m_sock, (SOCKADDR*)&m_addr, sizeof(sockaddr_in)) == SOCKET_ERROR) {
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            return false;
        }

        if (listen(m_sock, 5) == SOCKET_ERROR) {
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            return false;
        }

        m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
        if (m_hIocp == NULL) {
            m_hIocp = INVALID_HANDLE_VALUE;
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            return false;
        }
        CreateIoCompletionPort((HANDLE)m_sock, m_hIocp, (ULONG_PTR)this, 0);


        threadPool.Invoke();
        // ÿ�δ���һ��servSock���ͻ����threadIocp�����ն˿���ɡ�
        threadPool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&CMyServer::threadIocp));
        if (!NewAccept()) return false;
        return true;
    }

    bool NewAccept() {
        PCLIENT pClient(new MyClient());
        pClient->SetOverlapped(pClient);

        m_clients.insert(std::pair<SOCKET, PCLIENT>(*pClient, pClient));        // ? �����

        // AcceptEx
        // ����ͻ��������ӽ���ʱ���������ݣ���Щ���ݻᱻ�洢�� lpOutputBuffer �������С�
        // �����ͨ�� GetAcceptExSockaddrs ��������ȡ�ͻ��˺ͷ������ĵ�ַ��Ϣ
        // pClient �ᴫ������
        if (!AcceptEx(m_sock, *pClient, *pClient, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, *pClient, *pClient)) {
            m_hIocp = INVALID_HANDLE_VALUE;
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            return false;
        }
        return true;
    }

private:
    int threadIocp() {
        LPOVERLAPPED lpOverlapped = NULL;
        DWORD transferred = 0;
        ULONG_PTR key = 0;

        if(GetQueuedCompletionStatus(m_hIocp, &transferred, &key, &lpOverlapped, INFINITE))
        {
            if (transferred != 0 && (key != 0))
            {
                CBaseOverlapped* pOverlapped = CONTAINING_RECORD(lpOverlapped, CBaseOverlapped, m_overlapped);
                switch (pOverlapped->m_operator) {
                case MAccept:
                {
                    ACCEPTOVERLAPPED* pOver = (ACCEPTOVERLAPPED*)pOverlapped;
                    threadPool.DispatchWorker(pOver->m_worker);
                }
                break;

                case MRecv:
                {
                    RECVOVERLAPPED* pOver = (RECVOVERLAPPED*)pOverlapped;
                    threadPool.DispatchWorker(pOver->m_worker);
                }
                break;

                case MSend:
                {
                    SENDOVERLAPPED* pOver = (SENDOVERLAPPED*)pOverlapped;
                    threadPool.DispatchWorker(pOver->m_worker);
                }
                break;

                case MError:
                {
                    ERROROVERLAPPED* pOver = (ERROROVERLAPPED*)pOverlapped;
                    threadPool.DispatchWorker(pOver->m_worker);
                }
                break;

                }

                return 0;
            }
            else {
                return -1;
            }
            
        }
    }

private:
    CMyThreadPool threadPool;
    SOCKET m_sock;
    HANDLE m_hIocp;

    std::map<SOCKET, std::shared_ptr<MyClient>> m_clients;
    sockaddr_in m_addr;
};

