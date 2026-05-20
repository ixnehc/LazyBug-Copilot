/********************************************************************
	created:	2007/8/24   15:52
	filename: 	e:\IxEngine\Proj_GuiLib\WEditorPanel_Acl.cpp
	author:		cxi
	
	purpose:	asset class lib edit panel
*********************************************************************/
#include "stdh.h"
 
#include <vector>
#include <string>

#include "stringparser/stringparser.h"

#include "WorldEditorDefines.h"
#include "WEditorPanel_Acl.h"


#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IWorldSystem.h"

#include "WndBase.h"
#include "CommonCtrlBase.h"

#include "FileDialogBase.h"
#include ".\weditorpanel_acl.h"

//should be sychronized with the value in assetclasslib.cpp
#define NODETYPE_FOLDER 1
#define NODETYPE_CLASS_BEGIN 2
#define NODETYPE_CLASS_END 1002
#define NODETYPE_ISCLASS(type) (((type)>=NODETYPE_CLASS_BEGIN)&&((type)<=NODETYPE_CLASS_END))


//////////////////////////////////////////////////////////////////////////
//CAclTree
UINT CAclTree::_GetImageID()
{
	return IDB_RESTREEICON;

}

DWORD CAclTree::_GetImageIdx(NodeType type)
{
	if (type==NODETYPE_FOLDER)
		return 0;
	if (NODETYPE_ISCLASS(type))
		return 1;
	assert(FALSE);
	return 0;
}



//////////////////////////////////////////////////////////////////////////
//CEditorPanel_Acl


BEGIN_MESSAGE_MAP(CEditorPanel_Acl, CEditorPanel)
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()


CEditorPanel_Acl::CEditorPanel_Acl(CWnd* pParent):CEditorPanel(CEditorPanel_Acl::IDD, pParent)
{
}



BOOL CEditorPanel_Acl::OnInitDialog()
{
	CEditorPanel::OnInitDialog();

	CRect rc;
	GetClientRect(&rc);
	_tree.Create(this,rc,1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CEditorPanel_Acl::OnDestroy()
{
	CEditorPanel::OnDestroy();

	// TODO: Add your message handler code here
}

void CEditorPanel_Acl::OnInitAgent()
{

}

void CEditorPanel_Acl::SetEnv(EditorEnv &env)
{
	_classlib=((WEditorEnv&)env).pAS->GetClassLib();
	_tree.SetNodeTree(_classlib->GetNodeTree());
}

void CEditorPanel_Acl::OnUpdateUI()
{
}

void CEditorPanel_Acl::OnSize(UINT nType, int cx, int cy)
{
	CEditorPanel::OnSize(nType, cx, cy);

	if (_tree.m_hWnd)
	{
		CRect rc;
		GetClientRect(&rc);
		ClientToScreen(&rc);

		_tree.SetWindowPos(NULL,0,0,rc.Width(),rc.Height(),SWP_NOMOVE|SWP_NOZORDER);
	}

}

BOOL CEditorPanel_Acl::OnEvent(EditorEvent &evnt)
{
	switch(evnt.eventid)
	{
		case WEE_GetSelAssetClass:
		{
			if (!_classlib)
				return FALSE;
			IAssetClass *cls=NULL;
			CNodeTree *ntree=_classlib->GetNodeTree();
			NodeHandle hSel=_tree.GetCurSel();

			if (hSel)
			{
				std::string path=ntree->GetPath(hSel);
				cls=_classlib->ObtainClass(path.c_str());
			}

			evnt.ret=(DWORD)cls;
			return TRUE;
		}

	}

	return FALSE;
}
