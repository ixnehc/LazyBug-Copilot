#pragma once

#include "GuiLib.h"
#include "fastdelegate/FastDelegate.h"

#define GET_CONTROL_RECT(pDlg,id,rc)\
{\
	CWnd *pWnd;\
	pWnd=(pDlg)->GetDlgItem(id);\
	pWnd->GetWindowRect(&(rc));\
	(pDlg)->ScreenToClient(&(rc));\
}

#define HIDE_CONTROL(pDlg,id)\
{\
	CWnd *pWnd;\
	pWnd=(pDlg)->GetDlgItem(id);\
	if (pWnd)\
		pWnd->ShowWindow(SW_HIDE);\
}

#define SHOW_CONTROL(pDlg,id)\
{\
	CWnd *pWnd;\
	pWnd=(pDlg)->GetDlgItem(id);\
	if (pWnd)\
	pWnd->ShowWindow(SW_SHOW);\
}


#define ENABLE_CONTROL(pDlg,id)\
{\
CWnd *pWnd;\
pWnd=(pDlg)->GetDlgItem(id);\
if (pWnd)\
pWnd->EnableWindow(TRUE);\
}

#define DISABLE_CONTROL(pDlg,id)\
{\
CWnd *pWnd;\
pWnd=(pDlg)->GetDlgItem(id);\
if (pWnd)\
pWnd->EnableWindow(FALSE);\
}

#define CHECK_BUTTON(pDlg,id,bCheck)												\
{																													\
	CButton *pWnd;																						\
	pWnd=(CButton *)(pDlg)->GetDlgItem(id);											\
	if (pWnd)																									\
	{																												\
		if ((bCheck))																						\
			pWnd->SetCheck(1);																		\
		else																										\
			pWnd->SetCheck(0);																		\
	}																												\
}


BOOL IS_BUTTON_CHECK(CWnd *pDlg,UINT id);


#define ATTACH_CONTROL(pDlg,ctrlobj,id)\
{\
	CWnd *pWnd;\
	pWnd=(pDlg)->GetDlgItem(id);\
	if (pWnd)\
		(ctrlobj).Attach(pWnd->m_hWnd);\
}

#define DETACH_CONTROL(ctrlobj) (ctrlobj).Detach()

#define LOCATE_CONTROL(pDlg,id,rc)\
{\
	CWnd *pWnd;\
	pWnd=(pDlg)->GetDlgItem(id);\
	if (pWnd)\
		::SetWindowPos(pWnd,rc);\
}


#define SET_CONTROL_TEXT(pDlg,id,str)		\
{	\
	CWnd *pWnd;\
	pWnd=(pDlg)->GetDlgItem(id);\
	if (pWnd)\
	{	\
		CString s;	\
		pWnd->GetWindowText(s);	\
		if (!(s==(str)))	\
			pWnd->SetWindowText(str);	\
	}	\
}



GuiLib_Api BOOL CreateImageList(CImageList& il, UINT nID,int w,int h);

//Get the clipped rect of the given window
GuiLib_Api BOOL GetWndClippedRect(CWnd *pWnd,CRect &rc,BOOL bConsiderChildren=TRUE);

GuiLib_Api BOOL CheckWndDescendant(CWnd *pWnd,CWnd *pWndToCheck);

GuiLib_Api CWnd *GetMostAscendent(CWnd *pWnd);//得到顶级的父窗口


GuiLib_Api void SetWindowPos(CWnd *pWnd,i_math::recti &rc);

