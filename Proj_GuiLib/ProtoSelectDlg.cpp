/********************************************************************
	created:	2008/5/7   16:06
	file path:	d:\IxEngine\Proj_GuiLib
	author:		cxi
	
	purpose:	dialog for selecting proto
*********************************************************************/


#include "stdh.h"
#include "resource.h"
#include "ProtoSelectDlg.h"

#include "WndBase.h"
#include ".\protoselectdlg.h"

#include "WorldSystem/IEntitySystem.h"

#include "stringparser/stringparser.h"

//////////////////////////////////////////////////////////////////////////
//CProtoSelTree

BEGIN_MESSAGE_MAP(CProtoSelTree, CNodeTreeCtrl)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
END_MESSAGE_MAP()


//should be sychronized with the value in protolib.cpp
#define NODETYPE_FOLDER 1
#define NODETYPE_PROTO 2

void CProtoSelTree::SetProtoLib(IProtoLib *lib)
{
	_lib=lib;
	CNodeTreeCtrl::SetNodeTree(_lib->GetNodeTree());
}


UINT CProtoSelTree::_GetImageID()
{
	return IDB_PRLTREEICON;

}

DWORD CProtoSelTree::_GetImageIdx(NodeHandle hNode,SscState state)
{
	if (!_Tree())
		return 0;
	NodeType type=_Tree()->GetType(hNode);
	if (type==NODETYPE_FOLDER)
		return 0;
	if (type==NODETYPE_PROTO)
		return 1;

	return 0;
}

BOOL CProtoSelTree::_OnFilterItem(NodeHandle hNode)
{
	if (!_Tree())
		return FALSE;

	std::string path=_Tree()->GetPath(hNode);

	IProto *proto=_lib->ObtainProto(path.c_str());

	if (!proto)
		return FALSE;

	if ((_flag==1)&&(proto->IsLuaOnly()))
		return TRUE;

	if ((_flag==2)&&(!proto->IsLuaOnly()))
		return TRUE;

	return FALSE;

}

void CProtoSelTree::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	if (!_Tree())
		return;

	NodeHandle hNode=GetCurSel();
	if (hNode!=NodeHandle_Null)
	{
		NodeType type=_Tree()->GetType(hNode);
		if (type==NODETYPE_PROTO)
		{
			if (_owner)
				_owner->OnOK();
		}
	}
}





//////////////////////////////////////////////////////////////////////////
//CProtoSelectDlg

CProtoSelectDlg::CProtoSelectDlg(CWnd* pParent /*=NULL*/)
	: CXTPDialog(IDD_PROTOSELECTDLG, pParent)
{
	_lib=NULL;

	_bShowSelNone=FALSE;
}

void CProtoSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CProtoSelectDlg, CXTPDialog)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_SELNONE, OnBnClickedSelnone)
END_MESSAGE_MAP()


BOOL CProtoSelectDlg::Create(CWnd *pParent)
{
	return CDialog::Create(IDD_PROTOSELECTDLG,pParent); 
}

// CProtoSelectDlg 消息处理程序

TreeCtrlState CProtoSelectDlg::_state;
BOOL CProtoSelectDlg::_bForceReload=FALSE;

BOOL CProtoSelectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rc;
	GET_CONTROL_RECT(this,IDC_TREE,rc);
	HIDE_CONTROL(this,IDC_TREE);

	if (!_bShowSelNone)
		HIDE_CONTROL(this,ID_SELNONE);

	if (_bForceReload)
		_lib->Reload();

	_tree.Create(this,rc,IDC_TREE);

	_tree.SetProtoLib(_lib);
	_tree.EnableEdit(FALSE);
	_tree._owner=this;

	RestoreTreeCtrlState(&_tree,_state,"\\");

	if (TRUE)
	{
		std::vector<std::string> pieces;
		std::string s=_pathSel+".prt";
		
		HTREEITEM hItem=_tree.ItemFromPath(s);
		if (hItem)
		{
			_tree.SelectItem(hItem);
			_tree.EnsureVisible(hItem);
		}
	}

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CProtoSelectDlg::OnOK()
{
	_pathSel="";
	NodeHandle hSel=_tree.GetCurSel();
	if (hSel!=NodeHandle_Null)
	{
		if (_tree.GetNodeTree()->GetTree())
		{
			_pathSel=_tree.GetNodeTree()->GetTree()->GetPath(hSel);
			RemoveFileSuffix(_pathSel);
			_state.clear();
			RecordTreeCtrlState(&_tree,_state,"\\");
		}
	}
	CXTPDialog::OnOK();
}

void CProtoSelectDlg::OnCancel()
{
	CXTPDialog::OnCancel();
}

void CProtoSelectDlg::OnBnClickedSelnone()
{
	// TODO: Add your control notification handler code here
	_pathSel="";
	CXTPDialog::OnOK();
}
