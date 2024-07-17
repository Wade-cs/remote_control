# ���οؼ�

```cpp
//=====�ؼ�����=====

CTreeCtrl m_tree;

//===����������Ϣ===

//HTREEITEM �ڵ�ṹ��
HTREEITEM hTemp =m_tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);

m_tree.InsertItem("", hTemp, TVI_LAST);
			

//===ƴ��·���õ��ļ����Ե�ַ===
CString CRemoteClientDlg::GetPath(HTREEITEM hTree)
{
	CString strRet, strTemp;
	do {
		strTemp = m_tree.GetItemText(hTree);

		strRet = strTemp + "\\" +  strRet;
		
		hTree = m_tree.GetParentItem(hTree);

	} while (hTree != NULL);
	return strRet;
}

//������˫��ͬһ���ڵ㣬ÿ��˫��ǰ����������ڵ���ӽڵ㡣
void CRemoteClientDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{
	HTREEITEM hSub = NULL;
	do {
		hSub = m_tree.GetChildItem(hTree);
		if (hSub != NULL) m_tree.DeleteItem(hSub);
	} while (hSub != NULL);
}


//
void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
	
	// ��ȡ���������� --> ת����������� --> HitTest()������ָ��λ�ö��ڵ����ؼ���
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_tree.ScreenToClient(&ptMouse);
	HTREEITEM hTreeSelected = m_tree.HitTest(ptMouse, 0);	//����ȷ�����ָ��λ�ö�Ӧ�����ؼ��
	if (hTreeSelected == NULL) {
		return;
	}


	if (m_tree.GetChildItem(hTreeSelected) == NULL)	//�ļ���������������
		return;

	DeleteTreeChildrenItem(hTreeSelected);	//���

	CString strPath = GetPath(hTreeSelected);

	int nCmd = SendCommandPacket(2, false,(BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	
	PFILEINFO pFileInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	CClientSocket* pClient = CClientSocket::getInstance();

	while (pFileInfo->HasNext) {
		if (pFileInfo->IsDirectory) {
			if (CString(pFileInfo->szFileName) == "." || (CString(pFileInfo->szFileName) == ".."))
			{
				int cmd = pClient->DealCommand();
				TRACE("ack:%d\n", pClient->GetPacket().sCmd);
				if (cmd < 0) {
					break;
				}
				pFileInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
				continue;
			}
		}
		HTREEITEM hTemp = m_tree.InsertItem(pFileInfo->szFileName, hTreeSelected, TVI_LAST);
		if (pFileInfo->IsDirectory) {	//Ŀ¼�������һ���սڵ㣬���������ļ���
			m_tree.InsertItem("", hTemp, TVI_LAST);
		}
		int cmd = pClient->DealCommand();
		TRACE("ack:%d\n", pClient->GetPacket().sCmd);
		if (cmd < 0) {
			break;
		}
		  
		pFileInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}
	
	pClient->CloseSocket();
}
```