#pragma once
#include <map>
#include "ServerSocket.h"
#include <atlimage.h>
#include <direct.h>
#include "Tool.h"
#include <io.h>
#include <list>
#include "LockInfoDialog.h"
#include "resource.h"

class CCommand
{
public:
	CCommand();

	~CCommand(){
	}

	int ExcuteCommand(int nCmd);

protected:
	typedef int(CCommand::* CMDFUNC)();	//��Ա����ָ��
	std::map<int, CMDFUNC> m_mapFunction;
    CLockInfoDialog dlg;

    unsigned threadId;

protected:
    static unsigned __stdcall threadEntryForLockDlg(void* arg)
    {   
        CCommand* thiz = (CCommand*)arg;
        thiz->threadForLockDlg();
        _endthreadex(0);

        return 0;
    }

    void threadForLockDlg() {

        TRACE("[������]%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
        //��ģ̬
        dlg.Create(IDD_DIALOG_INFO, NULL);

        dlg.ShowWindow(SW_SHOW);

        //ȫ���ڱ�
        CRect rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
        rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);

        rect.bottom = LONG(rect.bottom * 1.08);

        dlg.MoveWindow(rect);

        // ��ʾ
        CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
        if (pText) {
            CRect rtText;
            pText->GetWindowRect(rtText);
            int nWidth = rtText.Width() / 2;
            int nHeight = rtText.Height() / 2;
            int x = (rect.right - nWidth) / 2;
            int y = (rect.bottom - nHeight) / 2;

            pText->MoveWindow(x, y, rtText.Width(), rtText.Height());
            pText->SetWindowText(_T("����ϵ����Ա����"));
        }

        //�����ö�
        dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

        //������깦��
        ShowCursor(false);

        //����������
        ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);

        //���������Χ
        ClipCursor(rect);

        MSG msg;

        //��Ϣ���߳���󶨡�
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_KEYDOWN) {
                TRACE("[������]msg:%08X wparam:%08X lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
                if (msg.wParam == 0x41) { //Esc��1B�� A��0x41��
                    break;
                }
            }
        }


        ClipCursor(NULL);
        //�ָ����
        ShowCursor(true);
        //�ָ�������
        ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);

        //�������û��Ӧ 
        dlg.DestroyWindow();
    }

    int MakeDriverInfo() {
        std::string result;
        for (int i = 1; i <= 26; i++) {
            if (_chdrive(i) == 0) {
                /*if (result.size() > 0) {
                    result += ',';
                }*/
                result += 'A' + i - 1;
                result += ',';
            }
        }

        CPacket pack(1, (BYTE*)result.c_str(), result.size());
        CTool::Dump((BYTE*)pack.Data(), pack.Size());

        CServerSocket::getInstance()->Send(pack);
        return 0;
    }

    int MakeDirectoryInfo() {
        std::string strPath;
        if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
            OutputDebugString(_T("��ǰ������ǻ�ȡ�ļ��б������������"));
            return -1;
        }

        if (_chdir(strPath.c_str()) != 0) {
            FILEINFO finfo;
            finfo.HasNext = FALSE;
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));

            CServerSocket::getInstance()->Send(pack);

            OutputDebugString(_T("û��Ȩ�ޣ�����Ŀ¼��"));
            return -2;
        }

        _finddata_t fdata;
        long long hfind = 0;
        if ((hfind = _findfirst("*", &fdata)) == -1) {
            OutputDebugString(_T("û���ҵ��κ��ļ���"));

            FILEINFO finfo;
            finfo.HasNext = FALSE;
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            CServerSocket::getInstance()->Send(pack);

            return -3;
        }
        int count = 0;
        do {
            FILEINFO finfo;
            finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
            memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
            TRACE("[������]%s HasNext=%d IsDirectory=%d IsInvalid=%d \r\n", finfo.szFileName, finfo.HasNext, finfo.IsDirectory, finfo.IsInvalid);
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            CServerSocket::getInstance()->Send(pack);
            count++;

        } while (!_findnext(hfind, &fdata));
        TRACE("[������]file_count = %d\n", count);

        //���ļ���Ϣ����ǽ�β��
        FILEINFO finfo;
        finfo.HasNext = FALSE;
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        if (CServerSocket::getInstance()->Send(pack) == false) {
            OutputDebugString(_T("����ʧ��\n"));
            return -4;
        }
        return 0;
    }

    int RunFile() {
        std::string strPath;
        if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
            OutputDebugString(_T("��ǰ������������ļ��������������"));
            return -1;
        }

        ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);

        CPacket pack(3, NULL, 0);
        if (CServerSocket::getInstance()->Send(pack) == false) {
            OutputDebugString(_T("����ʧ��"));
            return -2;
        }
        return 0;
    }

    //�����������ͻ��� �ļ���
    int DownloadFile() {
        std::string strPath;
        CServerSocket::getInstance()->GetFilePath(strPath);
        long long data = 0;     //�ļ�����
        //FILE* pFile = fopen(strPath.c_str(), "rb");          
        FILE* pFile = NULL;
        errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");   //��� _WINSOCK_DEPRECATED_NO_WARNINGS ����
        if (err != 0) {
            CPacket pack(4, (BYTE*)&data, 0);
            CServerSocket::getInstance()->Send(pack);
            return -1;
        }

        if (pFile != NULL) {
            //�����ļ���С��Ϣ�����ڼ��������ļ��Ľ�����Ϣ��
            fseek(pFile, 0, SEEK_END);
            data = ftell(pFile);
            CPacket head(4, (BYTE*)&data, 8);
            CServerSocket::getInstance()->Send(head);

            fseek(pFile, 0, SEEK_SET);

            char buffer[1024] = "";
            size_t rlen = 0;
            do {
                rlen = fread(buffer, 1, 1024, pFile);
                CPacket pack(4, (BYTE*)&buffer, rlen);
                CServerSocket::getInstance()->Send(pack);
            } while (rlen >= 1024);

            fclose(pFile);
        }

        //��ǽ���
        CPacket pack(4, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
        return 0;
    }

    int MouseEvent() {
        MOUSEEV mouse;
        if (CServerSocket::getInstance()->GetMouseEvent(mouse) == true) {
            WORD nFlags = 0;

            //ȷ������switch�����if�ж����
            switch (mouse.nButton)
            {
            case 0:     //���
                nFlags = 1;
                break;
            case 1:     //�Ҽ�
                nFlags = 2;
                break;
            case 2:     //�м�
                nFlags = 4;
                break;
            case 3:     //û�а���
                nFlags = 8;
                break;
            }

            // ����ƶ���mouse_event
            SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);


            switch (mouse.nAction)
            {
            case 0: //���
                nFlags |= 0x10;
                break;
            case 1: //˫��
                nFlags |= 0x20;
                break;
            case 2: //����
                nFlags |= 0x40;
                break;
            case 3: //�ſ�
                nFlags |= 0x80;
                break;
            case 4:
                break;
            default:
                break;
            }

            switch (nFlags)
            {
            case 0x11://������
                mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x21://���˫��
                mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x41://�������
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x81://����ſ�
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
                break;

            case 0x12://�Ҽ����
                mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x22://�Ҽ�˫��
                mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x42://�Ҽ�����
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x82://�Ҽ��ſ�
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
                break;

            case 0x14://�м����
                mouse_event(MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x24://�м�˫��
                mouse_event(MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
                mouse_event(MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x44://�м�����
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
                break;
            case 0x84://�м��ſ�
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
                break;

                //case 0x08://������ƶ�
                //    mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
                //    break;
            }

            //�������
            CPacket pack(5, NULL, 0);
            CServerSocket::getInstance()->Send(pack);
        }
        else {
            OutputDebugString(_T("��ȡ����������ʧ��"));
            return -1;
        }
        return 0;
    }

    int SendScreen()
    {
        CImage screen;  //GDI

        HDC hScreen = ::GetDC(NULL);
        int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
        int nWidth = GetDeviceCaps(hScreen, HORZRES);
        int nHeigth = GetDeviceCaps(hScreen, VERTRES);

        screen.Create(nWidth, nHeigth, nBitPerPixel);

        BitBlt(screen.GetDC(), 0, 0, nWidth, nHeigth, hScreen, 0, 0, SRCCOPY);

        ReleaseDC(NULL, hScreen);

        //screen.Save(_T("test2021.png"), Gdiplus::ImageFormatPNG);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
        if (hMem == NULL) {
            return -1;
        }

        IStream* pStream = NULL;

        HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);

        if (ret == S_OK) {
            screen.Save(pStream, Gdiplus::ImageFormatPNG);

            LARGE_INTEGER bg = { 0 };
            pStream->Seek(bg, STREAM_SEEK_SET, NULL);

            PBYTE pData = (PBYTE)GlobalLock(hMem);

            SIZE_T nSize = GlobalSize(hMem);

            CPacket pack(6, pData, nSize);
            CServerSocket::getInstance()->Send(pack);

            GlobalUnlock(hMem);
        }

        pStream->Release();
        GlobalFree(hMem);
        screen.ReleaseDC();

        return 0;
    }

    //���һ�� Dialog����ʽPopup�� ϵͳ�˵�False���߿�None

    //�����̣߳�����ϵͳ��Ӧ������Ϣ��
    int LockMachine()
    {
        if (dlg.m_hWnd == NULL || dlg.m_hWnd == INVALID_HANDLE_VALUE) {
            //_beginthread(threadLockDig, 0, NULL);
            _beginthreadex(NULL, 0, &CCommand::threadEntryForLockDlg, this, 0, &threadId);
            TRACE("[������]%s(%d):%d\r\n", __FUNCTION__, __LINE__, threadId);
        }

        CPacket pack(7, NULL, 0);
        CServerSocket::getInstance()->Send(pack);

        return 0;
    }

    int UnlockMachine()
    {
        //dlg.SendMessage(WM_KEYDOWN, 0x41, 001E0001);
        //::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x41, 001E0001);
        //������ winapi����main����һ���̡߳���Ҫ ���ض����߳� ����Ϣ��
        PostThreadMessage(threadId, WM_KEYDOWN, 0x41, 001E0001);

        CPacket pack(8, NULL, 0);
        CServerSocket::getInstance()->Send(pack);

        return 0;
    }

    int DeleteLocalFile()
    {
        std::string filePath;
        if (CServerSocket::getInstance()->GetFilePath(filePath) == false)
        {
            OutputDebugString(_T("��ǰ�������ɾ���ļ��������������"));
            return -1;
        }

        DeleteFile(filePath.c_str());

        //�ɹ��󣬻�Ӧ��
        CPacket pack(9, NULL, 0);
        if (CServerSocket::getInstance()->Send(pack) == false) {
            OutputDebugString(_T("����ʧ��"));
            return -2;
        }
        return 0;
    }

    int TestConnect()
    {
        CPacket pack(2024, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
        return 0;
    }

};

