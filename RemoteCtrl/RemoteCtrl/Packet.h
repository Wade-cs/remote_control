#pragma once
#pragma pack(push)
#pragma pack(1)	//����һ�ֽڶ��룬���CC������

#include "pch.h"
#include "framework.h"

class CPacket
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}

	//nSize ���ݴ�С
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}

		sSum = 0;
		for (int j = 0; j < nSize; j++) {
			sSum += BYTE(pData[j]) & 0xFF;
		}
	}

	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}

	//nSize���������ݴ�С�������ǻ�ȡ��������
	CPacket(const BYTE* pData, size_t& nSize) {
		size_t i = 0;
		//����pData���ҵ���0xFEFF��ͷ�İ�ͷ��
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		//���Ե���nSize��ֻ�п�������ް����ݡ�
		if (i + 4 + 2 + 2 > nSize) {	//�����ݿ��ܲ�ȫ�����߰�ͷδ��ȫ�����յ���
			nSize = 0;
			return;
		}

		nLength = *(DWORD*)(pData + i);
		i += 4;
		if (i + nLength > nSize) {		//����û�жԴ����ݰ����� ���Ͷ� �ְ�����nLength��¼���������ĳ��ȡ�
			nSize = 0;
			return;
		}

		sCmd = *(WORD*)(pData + i);
		i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}

		sSum = *(WORD*)(pData + i);
		i += 2;

		WORD temp = 0;
		for (int j = 0; j < strData.size(); j++) {
			temp += BYTE(strData[j]) & 0xFF;
		}

		if (temp == sSum) {
			//nSize = nLength + 2 + 4;
			nSize = i;						//��ͷǰ���������Ч���ݣ��ü�¼ȫ������������
			return;
		}
		nSize = 0;
	}

	~CPacket() {}

	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	int Size() {	//���ݵĴ�С
		return nLength + 6;
	}

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
	//unsigned short	WORD
	//unsigned long		DWORD

	WORD sHead;		//�̶�Ϊ��0xFEFF							2
	DWORD nLength;	//������ �ӿ������ʼ�����ͼ������		4
	WORD sCmd;		//��������								2
	std::string strData;	//������							
	WORD sSum;		//��У��	 У�������						2
	std::string strOut;
};
#pragma pack(pop)

typedef struct MouseEvent {
	MouseEvent() {
		nAction = -1;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;	//���0��˫��1������2���ſ�3
	WORD nButton;	//���0���Ҽ�1���м�2
	POINT ptXY;		//����
}MOUSEEV, * PMOUSEEV;

typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;         //�Ƿ���Ч
	BOOL IsDirectory;       //�Ƿ�ΪĿ¼ 0�� 1�� -1��Ч��Ĭ�ϣ�
	BOOL HasNext;           //�Ƿ��к��� 0û�� 1�У�Ĭ�ϣ�
	char szFileName[256];   //�ļ��� 
}FILEINFO, * PFILEINFO;
