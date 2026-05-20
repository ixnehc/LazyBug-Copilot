



#include "stdh.h"
#include "ValueSetEdit.h"
#include "Resource.h"

CValueSetEditor::CValueSetEditor()
{
} 

CValueSetEditor::~CValueSetEditor()
{

}

// 这里要考虑重复创建的问题
BOOL CValueSetEditor::Create( RECT &rc, CWnd *pParentWnd ,UINT id)
{	
	CWnd::CreateEx(0, _T("LISTBOX"), _T(""), WS_CHILD | WS_VISIBLE,
		rc, pParentWnd, id);
	return TRUE;
}


BOOL CValueSetEditor::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message==WM_MBUTTONDOWN)||(pMsg->message==WM_RBUTTONDOWN))
	{
		SetFocus();
	}
	return CGuiViewWnd<CWnd>::PreTranslateMessage(pMsg);
}
