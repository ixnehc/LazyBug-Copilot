/********************************************************************
	created:	2007/8/28   9:11
	filename: 	e:\IxEngine\Proj_GuiLib\WEditorPanel_Ap.cpp
	author:		cxi
	
	purpose:	Asset Property Edit Panel
*********************************************************************/
#include "stdh.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"

#include "WEditorPanel_Ap.h"


#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IWorldSystem.h"

#include "WndBase.h"
#include "CommonCtrlBase.h"

#include "WorldEditorDefines.h"

#include "gds/GObj.h"
#include ".\weditorpanel_ap.h"

//////////////////////////////////////////////////////////////////////////
//CAssetClassPage
BEGIN_MESSAGE_MAP(CAssetClassPage, CGObjGrid)
END_MESSAGE_MAP()

void CAssetClassPage::Reset()
{
	CGObjGrid::Bind(NULL);
	Zero();
}


void CAssetClassPage::_Bind()
{
	std::vector<GObjBase *>objs;
	DWORD sz=0;
	if (_cls)
		sz=_cls->GetDataCount();
	objs.resize(sz);
	for (int i=0;i<sz;i++)
		objs[i]=_cls->GetData(i);

	CGObjGrid::Bind(&objs[0],sz);

	CGObjGrid::ExpandAll();

	_bReadOnly=FALSE;

}

void CAssetClassPage::Bind(IAssetClassLib *classlib,IAssetClass *cls)
{
	if ((_cls==cls)&&(_classlib==classlib))
		return;

	_cls=cls;
	_classlib=classlib;

	_Bind();

}


BOOL CAssetClassPage::Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle)
{
	if (!CGObjGrid::Create(rect,pParentWnd,nID,dwListStyle))
		return FALSE;

	ShowToolBar(FALSE);
	return TRUE;

}

void CAssetClassPage::OnEndItemChange(CXTPPropertyGridItem *item)
{
	CGObjGrid::OnEndItemChange(item);

	_classlib->SetModified();
}




//////////////////////////////////////////////////////////////////////////
//CEditorPanel_Ap


BEGIN_MESSAGE_MAP(CEditorPanel_Ap, CEditorPanel)
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()


CEditorPanel_Ap::CEditorPanel_Ap(CWnd* pParent):CEditorPanel(CEditorPanel_Ap::IDD, pParent)
{
	_cls=NULL;
	_page=NULL;
}



BOOL CEditorPanel_Ap::OnInitDialog()
{
	CEditorPanel::OnInitDialog();

	CRect rc(0,0,1,1);
	_GetPage()->Create(rc,this,1);

	_RefreshClass();
	_Arrange();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CEditorPanel_Ap::OnDestroy()
{
	SAFE_DELETE(_page);

	CEditorPanel::OnDestroy();

	// TODO: Add your message handler code here
}

void CEditorPanel_Ap::OnInitAgent()
{

}

void CEditorPanel_Ap::SetEnv(EditorEnv &env)
{
	_classlib=((WEditorEnv&)env).pAS->GetClassLib();
}


void CEditorPanel_Ap::OnUpdateUI()
{
	BOOL bNeedArrange=FALSE;

	IAssetClass *cls=NULL;
	EditorEvent evnt(WEE_GetSelAssetClass);
	if (GetMgr()->SendEvent(evnt))
		cls=(IAssetClass *)evnt.ret;
	if (cls!=_cls)
	{
		_cls=cls;
		_RefreshClass();
		bNeedArrange=TRUE;
	}
	if (bNeedArrange)
		_Arrange();
}

void CEditorPanel_Ap::OnSize(UINT nType, int cx, int cy)
{
	CEditorPanel::OnSize(nType, cx, cy);

	_Arrange();

}



void CEditorPanel_Ap::_RefreshClass()
{
	_GetPage()->Bind(_classlib,_cls);
}

void CEditorPanel_Ap::_Arrange()
{
	if (_GetPage()->GetSafeHwnd())
	{
		CRect rc;
		GetClientRect(&rc);

		_GetPage()->SetWindowPos(NULL,0,0,rc.Width(),rc.Height(),SWP_NOZORDER);
	}
}

CAssetClassPage *CEditorPanel_Ap::_GetPage()
{
	if (!_page)
		_page=new CAssetClassPage;
	return _page;
}
