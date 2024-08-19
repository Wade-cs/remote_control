#pragma once
#include "MyThread.h"
#include "pch.h"
#include <map>
#include <MSWSock.h>
#include "Tool.h"
#include "MyQueue.h"

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
        memset(&m_wsaBuffer, 0, sizeof(WSABUF));
    }

    int AcceptWorker() {
        // TODO:
        INT lLength = 0, rLength = 0;
        if (m_pClient->GetBufferSize() > 0) {       // m_pClient->GetBufferSize() > 0 ��ζ�� ֮ǰ AceeptEx ���յ� client ��Ϣ
            LPSOCKADDR pLocalAddr, pRemoteAddr;
            GetAcceptExSockaddrs((PVOID)*m_pClient, 0,
                sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
                (sockaddr**)&pLocalAddr, &lLength,//���ص�ַ
                (sockaddr**)&pRemoteAddr, &rLength//Զ�̵�ַ
            );

            memcpy(m_pClient->GetLocalAddr(), pLocalAddr, sizeof(sockaddr_in));
            memcpy(m_pClient->GetRemoteAddr(), pRemoteAddr, sizeof(sockaddr_in));

            // AcceptEx�õ��Ŀͻ����׽��� �� iocp
            m_server->BindNewSocket(*m_pClient);

            // WSARecv ֪ͨ IOCP �����Ϣ��֮����ȥ recv�����ﲢû��ʹ�õ� WSABUF��
            int ret = WSARecv((SOCKET)*m_pClient,  m_pClient->RecvWSABuf(), 1, *m_pClient, &m_pClient->flags(), m_pClient->RecvOverlapped(), NULL);
            if (ret == SOCKET_ERROR && (WSAGetLastError() != WSA_IO_PENDING)) {
                //TODO������
                TRACE("ret = %d error = %d\r\n", ret, GetLastError());
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

        memset(&m_wsaBuffer, 0, sizeof(WSABUF));
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
        memset(&m_wsaBuffer, 0, sizeof(WSABUF));
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
        memset(&m_wsaBuffer, 0, sizeof(WSABUF));
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
        m_isBusy(false), m_flags(0), m_received(0), m_usedIndex(0),
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
        return (PVOID)m_buffer.data();
    }

    operator LPDWORD() {
        return &m_received;
    }

    operator LPOVERLAPPED() {           // AcceptEx overlapped�á� Accept����ģ�ͨ��ǿת��ȡ��Ӧ��overlapped��recv��send������ķ������ء�
        return &m_overlapped->m_overlapped;
    }
    
    LPWSABUF RecvWSABuf() {
        return &m_recvOverlapped->m_wsaBuffer;
    }

    LPWSAOVERLAPPED RecvOverlapped() {
        return &m_recvOverlapped->m_overlapped;
    }

    LPWSABUF SendWSABuf() {
        return &m_sendOverlapped->m_wsaBuffer;
    }

    LPWSAOVERLAPPED SendOverlapped() {
        return &m_sendOverlapped->m_overlapped;
    }

    sockaddr_in* GetLocalAddr() { return &m_laddr; }
    sockaddr_in* GetRemoteAddr() { return &m_raddr; }
    size_t GetBufferSize() const { return m_buffer.size(); }
    DWORD& flags() { return m_flags; }

    int Recv() {
        int ret = recv(m_sock, m_buffer.data() + m_usedIndex, m_buffer.size() - m_usedIndex, 0);
        if (ret <= 0) {
            TRACE("error %d\r\n", GetLastError());
            return -1;
        }
        m_usedIndex += (size_t)ret;
        // TODO :��������
        CTool::Dump((BYTE*)m_buffer.data(), ret);
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
        WSACleanup();
    }

    bool StartService() {
        WSADATA WSAData;
        WSAStartup(MAKEWORD(2, 2), &WSAData);
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
        // ����һ���̣߳�ר������GetQueuedCompletionStatus����iocp��Ϣ��
        int ret = threadPool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&CMyServer::threadIocp));
        TRACE("Thread index =  %d\r\n", ret);
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
        TRACE("[������]%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
       
        // AcceptEx��ɺ󣬷���I/O��ɰ���IOCP�С�ͨ��GetQueuedCompletionStatus���Ի�ȡ����Ӧ��lpOverlapped��AcceptEx�����һ������������ʼ����ʱ���Ѿ�����Ϊ��SENDOVERLAPPED��
        
        // �������� AcceptEx ʱ�������������أ�������һ���첽���ܲ�����
        // �����ʱ�Ѿ����µ���������ȴ�����AcceptEx ��������ʼ���������
        // ���û���µ���������AcceptEx ��ȴ��µ��������󵽴
        
        // д�� m_buffer �����Ա
        if (!AcceptEx(m_sock, *pClient, *pClient, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, *pClient, *pClient)) {
            TRACE("%d\r\n", WSAGetLastError());
            if ((WSAGetLastError() != WSA_IO_PENDING)) {
                closesocket(m_sock);
                m_hIocp = INVALID_HANDLE_VALUE;
                m_sock = INVALID_SOCKET;
                return false;
            } 
        }
        return true;
    }


    void BindNewSocket(SOCKET s) {
        if (CreateIoCompletionPort((HANDLE)s, m_hIocp, (ULONG_PTR)this, 0) == NULL) {
            TRACE("error %d \r\n", GetLastError());
        }
    }

private:
    int threadIocp() {
        LPOVERLAPPED lpOverlapped = NULL;
        DWORD transferred = 0;
        ULONG_PTR key = 0;
        TRACE("[������]%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
        if(GetQueuedCompletionStatus(m_hIocp, &transferred, &key, &lpOverlapped, INFINITE))
        {  
            if (key != 0)
            {
                CBaseOverlapped* pOverlapped = CONTAINING_RECORD(lpOverlapped, CBaseOverlapped, m_overlapped);
                TRACE("pOverlapped->m_operator %d \r\n", pOverlapped->m_operator);

                pOverlapped->m_server = this;       // ���ָ��server��ָ�룬���ڷ����ڵ��� NewAccept()

                switch (pOverlapped->m_operator) {
                case MAccept:
                {
                    ACCEPTOVERLAPPED* pOver = (ACCEPTOVERLAPPED*)pOverlapped;
                    // eg: ��̳�����£�ָ��ת�������ƫ�
                    // ACCEPTOVERLAPPED �̳��� CBaseOverlapped �Լ� ThreadFuncBase
                    // pOver �� ָ�� CBaseOverlapped ��ͷ�ĵ�ַ��
                    // CBaseOverlapped::m_worker �ĳ�ʼ����ʱ����AcceptOverlapped��������� ThreadWorker ��ʼ������� thiz(this) ת�� ThreadFuncBase��thiz �� pOver��Ⱦ���ƫ�ơ�

                    // ���������	ThreadWorker(void* obj, FUNCTYPE f) :thiz((ThreadFuncBase*)obj), func(f) {}

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
            }
            else {
                return -1;
            }
        }
        return 0;
    }

private:
    CMyThreadPool threadPool;
    SOCKET m_sock;
    HANDLE m_hIocp;

    std::map<SOCKET, std::shared_ptr<MyClient>> m_clients;
    sockaddr_in m_addr;
};

