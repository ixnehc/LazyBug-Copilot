#include "stdh.h"
#include "ChangelistsEdit.h"
#include "Resource.h"

CChangelistsEditor::CChangelistsEditor()
{
} 

CChangelistsEditor::~CChangelistsEditor()
{

}

// 这里要考虑重复创建的问题
BOOL CChangelistsEditor::Create( RECT &rc, CWnd *pParentWnd ,UINT id)
{	
	CWnd::CreateEx(0, _T("LISTBOX"), _T(""), WS_CHILD | WS_VISIBLE,
		rc, pParentWnd, id);
	return TRUE;
}


BOOL CChangelistsEditor::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message==WM_MBUTTONDOWN)||(pMsg->message==WM_RBUTTONDOWN))
	{
		SetFocus();
	}
	return CGuiViewWnd<CWnd>::PreTranslateMessage(pMsg);
}
