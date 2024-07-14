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

