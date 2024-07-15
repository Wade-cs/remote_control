# ����
����-������-����ѡ��-��ڵ㣨mainCRTStartup�� ��ϵͳ�����ڣ�
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")

# ����ģʽ �����ࣨ��CHelper��
��C++�У����ڵ���ģʽ��ʵ�֣�ʹ��һ�������ࣨ��CHelper����ȷ�������������ȷ������������һ�ֳ������������������漰��̬�������Դ����ʱ���������Ϊʲô��Ҫhelper����Э��ɾ���������󣬶�����ֱ���ڶ�������Ҫʱ��������������
�����������ڹ������ս

    ȷ��������Ψһ��:
    ����ģʽҪ��������Ӧ�ó�����ֻ��һ��ʵ�����ڡ�����ζ�Ź��캯��������˽�еģ���ֹ�ⲿֱ�Ӵ���ʵ����

    ��ȷ�ش���������:
    ��������ʵ��ͨ����������ľ�̬��Ա�����У���getInstance()���������ȷ�����ʵ���ʱ���������ʵ���أ�

��������������

�������������ɳ���Ա��ʽ���á����ǵ����󳬳����������ʽɾ��ʱ�ɱ������Զ����õġ�
�ڵ���ģʽ�£�����`��������ͨ����ȫ�ֵĻ�̬�ģ���������������������������һ�¡�`
����ζ�����û������Ļ��ƣ�������������������������ڳ���������ֹ֮ǰ�����á�
CHelper������

CHelper������������������⡣CHelper��һ�����캯����һ���������������Ƿֱ���CHelper����Ĵ���������ʱ�����á�ͨ�����²��裬CHelperȷ���˵����������ȷ�������ڹ���

    ��������:
        CHelper�Ĺ��캯���е���CServerSocket::getInstance()����ᴴ���򷵻��Ѵ��ڵ�CServerSocketʵ����

    ���ٵ���:
        ��CHelper������������ڽ����������������˳�ʱ���������������ᱻ���á�
        ���������е���CServerSocket::releaseInstance()������ͷ�CServerSocket�ĵ���ʵ����

���ַ������ŵ��ǣ�

    �Զ�����:
    CHelper��������������������������Ȼͬ����ȷ���˵����Ĵ�������������ȷ��ʱ�����С�

    ��ֹ�ڴ�й©:
    ��ʹ����Ա������ʽ�����ͷź�����CHelperҲ���ڳ����˳�ʱ�Զ����ã��������ڴ�й©�ķ��ա�

    �������:
    ����ģʽ�����ر����˵���������������ڹ�����ԣ�ʹ�ô�����ӽ�׳������ά����

�ܽ�

ʹ��CHelper����������������������ڣ���C++��һ�����ŵĽ����������ȷ������Դ����ȷ�ͷţ�ͬʱ��������������������ʽ���ô��������⡣���ַ���������������Щ��Ҫ�ڳ�������ʱ��ʼ�����ڳ������ʱ�������Դ�ܼ��Ͷ��������ݿ����ӻ������׽��֡�


# ���ݽ������ڶ�� CC �ڴ�������⡣
```cpp
#pragma pack(push)
#pragma pack(1)	//����һ�ֽڶ��룬���CC������
class Packet
{
    ...
    ...
};

#pragma pack(pop)
```
��ֹ�ڴ���롣

# (const char*)&pack ������

�� Packet ���ʵ��ת��Ϊ BYTE* ָ�룬��ζ�����뽫���� Packet ������ڴ沼�ֱ�ƽ��Ϊһ���ֽ����顣
�����������ж�����;�������������ϴ������ݻ��߽�����д���ļ���
Ȼ�������� Packet ����һ�� std::string ��Ա���������������Դ� POD��Plain Old Data���ṹ������ֱ�ӡ�

std::string ���ڲ������������֣�ָ���ַ������ݵ�ָ�롢�ַ����Ĵ�С�Լ��ַ�����������
����ζ�� std::string ������һ���򵥵������ڴ�飬���ܼ򵥵�ͨ��ȡ��ַ�ķ�ʽ����ת��Ϊһ���ֽ�ָ�벢��֤���л���ȷ��

���ܵõ����� std::stringԪ�صĵ�ַ��

```cpp
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
    std::string strOut;
```

# char ���鸳ֵ
```cpp
    memcpy(finfo->szFileName, fdata.name, strlen(fdata.name));
```




# API�ʼ�

## OutputDebugString();
OutputDebugString ��һ�� Windows API ��������Ҫ�����ڵ��Թ����������������ı���Ϣ��

## _chdir()  
��������������������ʱ�л�Ŀ¼������ڷ��ʲ�ͬλ�õ��ļ���Ŀ¼�ǳ����á�
```cpp
int _chdir(
   const char *path
);
```

## _finddata_t �ṹ��
```cpp
struct _finddata_t {
    unsigned int attrib;       // �ļ�����
    size_t size;               // �ļ���С
    time_t time_create;        // ����ʱ��
    time_t time_access;        // �ϴη���ʱ��
    time_t time_write;         // �ϴ��޸�ʱ��
    char name[260];            // �ļ���
};
```

> attrib �ļ�����
> _A_NORMAL: �ļ�������û���������ԣ�0x00��
> _A_RDONLY: �ļ�ֻ����0x01��
> _A_HIDDEN: �ļ����أ�0x02��
> _A_SYSTEM: �ļ���ϵͳ�ļ���0x04��
> _A_VOLID: ͨ�����ڱ�ʾ�ļ����ٴ����ھ��ϣ�0x08�������˱�־��ʵ�ʺ���������ļ�ϵͳ����
> _A_SUBDIR: �ļ���һ����Ŀ¼��0x10��
> _A_ARCH: �ļ��Ĵ浵���ԣ�ͨ����ʾ�ļ��ѱ��޸Ļ򴴽���0x20��


## _findfirst()   
�� Windows ƽ̨�������ļ�ö�٣������� Unix �е� glob() ������
```cpp
intptr_t _findfirst(
   const char *filename,     /* �ļ���ģʽ */                // "*" "*.txt" ...
   struct _finddata_t *fileinfo /* �����Ϣ�ṹ���ָ�� */
);
```

## _findnext()
��ʹ�� _findfirst ������ʼ�����������ö��Ŀ¼�е��ļ���������������������ǰĿ¼����ָ��ģʽƥ��������ļ���

_findnext ��ԭ�����£�
```cpp
int _findnext(
   intptr_t hFile,       // �ļ����Ҿ��
   struct _finddata_t *fileinfo // �ļ���Ϣ�ṹ���ָ��
);
```

## ShellExecuteA()
������ shell ���������Ĳ�����������ļ����ļ��л�������Ӧ�ó���

```cpp
HINSTANCE ShellExecuteA(
  HWND hwnd,     // �����ھ����ͨ��Ϊ NULL
  LPCSTR lpOperation, // ִ�еĲ������� "open"
  LPCSTR lpFile,      // �ļ������ִ���ļ���·��
  LPCSTR lpParameters,// ��������ѡ
  LPCSTR lpDirectory, // ����Ŀ¼����ѡ
  INT nShowCmd        // ��ʾ���ڵķ�ʽ
);
```

## �ļ�����

### fopen()  fopen_s()



### fseek()

### ftell()

### fread()

### fwrite()

### fclose()


## ������

### SetCursorPos()
���ڽ�������ƶ�����Ļ�ϵ�ָ��λ��

### mouse_event()

### GetMessageExtraInfo()

### �ƶ����
������Windows API��ʹ�� `SetCursorPos` �� `mouse_event` ���ƶ����ʱ����������������Ϊ��һЩ�ؼ��Ĳ�ͬ�㣺

`SetCursorPos`
```cpp
SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
```
- **����**�����д���������ƶ�����굽 `(mouse.ptXY.x, mouse.ptXY.y)` ָ��������λ�á�
- **�������¼�**��`SetCursorPos` ��������κ�����¼�������ζ�������ᱻ�κ�Ӧ�ó�����Ϊ�û����롣Ӧ�ó��򲻻���յ����ƶ���ص�WM_MOUSEMOVE��Ϣ��

`mouse_event`
```cpp
mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
```
- **����**�����д���ᷢ��һ������ƶ��¼���ϵͳ���·����û���Ȼ���ƶ�����ꡣ
- **�����¼�**��`mouse_event` ʹ�� `MOUSEEVENTF_MOVE` ��־��ģ������ƶ�������������ϵͳ����һ��WM_MOUSEMOVE��Ϣ�����Ӧ�ó������Ϊ����һ����ʵ���û����롣
- **������Ϣ**�����ݸ� `mouse_event` �����һ�������� `GetMessageExtraInfo()`����ͨ��������64λϵͳ�ϻ�ȡ���λ�õĸ�32λ����������ȷ���ڴ�ֱ�����Ļ���ƶ��¼���׼ȷ�ԡ�

`�����ܽ�`
- **�¼�����**��`SetCursorPos` ����������ƶ��¼����� `mouse_event` ������ɡ�
- **ϵͳ֪ͨ**��`SetCursorPos` ��֪ͨϵͳ��Ӧ�ó���������ƶ����� `mouse_event` ��ͨ���¼�֪ͨϵͳ��Ӧ�ó���
- **�û�����ģ��**��`SetCursorPos` ���ƶ���꣬û��ģ���û����룻`mouse_event` ����ȫģ���û��ƶ�������Ϊ��

���Զ������Ի�����Ҫģ���û������Ľű��У�����ܸ�������ʹ�� `mouse_event`����Ϊ���ܹ���׼ȷ��ģ���û���Ϊ���Ӷ�����Ӧ�ó����Ԥ����Ӧ��Ȼ���������ֻ����Ҫ�ı����λ�ã����������Ƿ񴥷�����¼���`SetCursorPos` ���㹻�ˡ�


## ������Ļ
```cpp
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

    //screen.Save(_T("test2020.png"), Gdiplus::ImageFormatPNG);
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
```

��ϸ����

1. **����`CImage`����**:
   ```cpp
   CImage screen;
   ```
   ����һ��`CImage`�����������ڴ洢��Ļ��ͼ��

2. **��ȡ��Ļ���豸������**:
   ```cpp
   HDC hScreen = ::GetDC(NULL);
   ```
   ʹ��`GetDC`������ȡ������Ļ���豸�����ģ�DC��������`NULL`��ʾ��ȡ������Ļ��DC��

3. **��ȡ��Ļ�ֱ��ʺ�λ���**:
   ```cpp
   int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
   int nWidth = GetDeviceCaps(hScreen, HORZRES);
   int nHeigth = GetDeviceCaps(hScreen, VERTRES);
   ```
   ͨ������`GetDeviceCaps`��������ȡ��Ļ��λ��ȣ�ÿ���ص�λ��������Ⱥ͸߶ȡ�

4. **����λͼ**:
   ```cpp
   screen.Create(nWidth, nHeigth, nBitPerPixel);
   ```
   ʹ��`CImage`�����`Create`��������һ���µ�λͼ����ߴ����ɫ�������Ļƥ�䡣

5. **������Ļ���ݵ�λͼ**:
   ```cpp
   BitBlt(screen.GetDC(), 0, 0, nWidth, nHeigth, hScreen, 0, 0, SRCCOPY);
   ```
   ʹ��`BitBlt`����������Ļ�����ݸ��Ƶ�ǰ�洴����λͼ�С�

6. **�ͷ���Ļ��DC**:
   ```cpp
   ReleaseDC(NULL, hScreen);
   ```
   �ͷ�֮ǰ��ȡ����ĻDC��������Դй©��

7. **�����ڴ�ʹ�����**:
   ```cpp
   HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
   IStream* pStream = NULL;
   HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
   ```
   ʹ��`GlobalAlloc`����������ƶ���ȫ���ڴ档���ţ�ʹ��`CreateStreamOnHGlobal`��������һ������ȫ���ڴ��������

8. **����λͼ����**:
   ```cpp
   screen.Save(pStream, Gdiplus::ImageFormatPNG);
   ```
   ʹ��`CImage`�����`Save`������λͼ����ΪPNG��ʽ��֮ǰ���������С�

9. **��λ��������ʼλ��**:
   ```cpp
   LARGE_INTEGER bg = { 0 };
   pStream->Seek(bg, STREAM_SEEK_SET, NULL);
   ```
   ʹ��`Seek`����������λ���ƶ�����ʼλ�á�

10. **��ȡ�ڴ��ַ�ʹ�С**:
    ```cpp
    PBYTE pData = (PBYTE)GlobalLock(hMem);
    SIZE_T nSize = GlobalSize(hMem);
    ```
    ʹ��`GlobalLock`��������֮ǰ������ڴ棬��ȡ�ڴ����ʼ��ַ�ʹ�С��

11. **�������ݰ�������**:
    ```cpp
    CPacket pack(6, pData, nSize);
    CServerSocket::getInstance()->Send(pack);
    ```
    ����һ��`CPacket`���󣬷�װPNGͼ�����ݣ�Ȼ��ʹ��`CServerSocket`��`Send`��������������ݰ���

12. **�ͷ���Դ**:
    ```cpp
    GlobalUnlock(hMem);
    pStream->Release();
    GlobalFree(hMem);
    ```
    �������ͷ�֮ǰ�����ȫ���ڴ棬�ͷ�������

13. **�ͷ�`CImage`��DC**:
    ```cpp
    screen.ReleaseDC();
    ```
    ����`CImage`��`ReleaseDC`���������ǣ�ͨ��`CImage`���Զ�������DC�������һ�����ܲ��Ǳ���ġ�

14. **����**:
    ```cpp
    return 0;
    ```
    �����ɹ�ִ�к󷵻�0��

ȷ����ʵ��ʹ���д���ô�������������`GlobalAlloc`ʧ�ܵ�������Լ���`CServerSocket::Send`�п��ܳ��ֵ�������󡣴��⣬���ڶ���ʾ��ϵͳ���������Ҫ�޸Ĵ���������ÿ����ʾ����������ֻ��������ʾ������Ļ��