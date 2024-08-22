#pragma once
class CTool
{
public:
	static void Dump(BYTE* pData, size_t nSize);


    // ����������ʱ�򣬳����Ȩ���Ǹ��������û���
    // �������Ȩ�޲�һ�£���ᵼ�³�������ʧ��
    // ���������Ի���������Ӱ�죬�������dll(��̬��)���������ʧ��
    // ��������Щdll��system32�������syswow64���桿
    // system32 �������64λ���� syswow64�������32λ����
    // ��ʹ�þ�̬�⣬���Ƕ�̬�⡿


    //���ÿ����������޸�ע���ʽ(��¼����������) 
    //��������ע���λ�ã������
    static int WriteRefisterTable(const CString strPath) {
        if (PathFileExists(strPath))return 0;//ע������Ѿ�����
        
        // ����̬�����ɣ����Բ���Ҫ�������ˣ�ֱ�Ӹ����ļ���
        char exePath[MAX_PATH] = "";
        GetModuleFileName(NULL, exePath, MAX_PATH);
        bool ret = CopyFile(exePath, strPath, FALSE);
        if (ret == FALSE) {
            MessageBox(NULL, TEXT("�����ļ���ʧ�ܣ��Ƿ�Ȩ�޲��㣿\r\n"), TEXT("����"), MB_ICONERROR | MB_TOPMOST);
            return -1;
        }

        //��ע���������λ��
        CString strSubKey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";//ע���������·��
        HKEY hKey = NULL;
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
        if (ret != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            MessageBox(NULL, TEXT("��ע���ʧ��! �Ƿ�Ȩ�޲���?\r\n"), TEXT("����"), MB_ICONERROR | MB_TOPMOST);
            return -2;
        }

        //����ִ���ļ���������ӵ�ע���������·����
        ret = RegSetValueEx(hKey, TEXT("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
        if (ret != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            MessageBox(NULL, TEXT("ע�������ļ�ʧ�� �Ƿ�Ȩ�޲���?\r\n"), TEXT("����"), MB_ICONERROR | MB_TOPMOST);
            return -3;
        }
        RegCloseKey(hKey);
        return 0;
    }

    //д���������ļ���ʽ
    static int WriteStartupDir(const CString& strPath)
    {
        if (PathFileExists(strPath))return 0; // �����ļ��Ѿ�����

        CString strCmd = GetCommandLine();
        TRACE("strCmd:%s \r\n", strCmd);        // ����exe����ľ���·��
        strCmd.Replace(TEXT("\""), TEXT(""));
        BOOL ret = CopyFile(strCmd, strPath, FALSE);
        if (ret == FALSE)
        {
            MessageBox(NULL, TEXT("�����ļ���ʧ�ܣ��Ƿ�Ȩ�޲���?\r\n"), TEXT("����"), MB_ICONERROR | MB_TOPMOST);
            return -1;
        }
        return 0;
    }


    static void ShowError() {
        LPCSTR lpMessageBuf = NULL;
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&lpMessageBuf, 0, NULL);
        OutputDebugString(lpMessageBuf);
        LocalFree((void*)lpMessageBuf);
    }

    static bool IsAdmin() {
        HANDLE hToken = NULL;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken));
        {
            ShowError();
            return false;
        }
        TOKEN_ELEVATION eve;
        DWORD len = 0;
        if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE) {
            ShowError();
            return false;
        }
        CloseHandle(hToken);
        if (len == sizeof(eve)) {
            return eve.TokenIsElevated;
        }

        printf("length of tokenInfomation is %d \r\n", len);
        return false;
    }


    static bool RunAsAdmin()
    {
        //TODO ��ȡ����ԱȨ�ޣ�ʹ�ø�Ȩ�޴������̡�

        WCHAR sPath[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, sPath, MAX_PATH);

        STARTUPINFOW si = { 0 };
        si.cb = sizeof(STARTUPINFOW);
        PROCESS_INFORMATION pi = { 0 };

        BOOL ret = CreateProcessWithLogonW((LPCWSTR)(LPWSTR)"Administrator", NULL, NULL, LOGON_WITH_PROFILE,
            NULL, sPath, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);

        if (!ret) {
            MessageBox(NULL, TEXT("��������ʧ��"), TEXT("�������"), MB_OK | MB_ICONERROR);
            ShowError();
            return false;
        }

        MessageBox(NULL, TEXT("���̴����ɹ�"), TEXT("�û�״̬"), 0);

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    static bool Init()
    {
        HMODULE hModule = ::GetModuleHandle(nullptr);

        if (hModule == nullptr)
        {
            // TODO: ���Ĵ�������Է�����Ҫ
            wprintf(L"����: GetModuleHandle ʧ��\n");
            return false;
        }

        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: �ڴ˴�ΪӦ�ó������Ϊ��д���롣
            wprintf(L"����: MFC ��ʼ��ʧ��\n");
            return false;
        }
        return true;
    }

};

