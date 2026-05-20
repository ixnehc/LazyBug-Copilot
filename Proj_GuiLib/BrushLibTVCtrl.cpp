
#include "stdh.h"

#include "BrushLibTVCtrl.h"

#include "stringparser/stringparser.h"

#include "SscBase.h"

#include "TreeCtrlBase.h"

DWORD CBrushLibTVCtrl::CBrushLibTreeCtrl::_GetImageIdx(NodeHandle hNode,SscState state)
{
	CNodeTree * nodetree =  GetNodeTree()->GetTree();
	if (!nodetree)
		return 0;
	DWORD itype = nodetree->GetType(hNode) - 1;
	return itype;
}

UINT CBrushLibTVCtrl::CBrushLibTreeCtrl:: _GetImageID()	
{
	return IDB_IMAGE_SPTLIB;	
}

void CBrushLibTVCtrl::CBrushLibTreeCtrl::PreSubclassWindow(){

	LONG_PTR Style = GetWindowLongPtr(GetSafeHwnd(),GWL_STYLE);
	Style |= WS_CHILD|WS_VISIBLE|WS_BORDER|
		TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|
		TVS_EDITLABELS;

	SetWindowLongPtr(GetSafeHwnd(),GWL_STYLE,(LONG)Style);

	CNodeTreeCtrl::PreSubclassWindow();
}

//////////////////////////////////////////////////////////////////////////

CBrushLibTVCtrl::CBrushLibTVCtrl(void)
{
	_pLib = NULL;
	_ssc = NULL;
	_ver = NULL;
}

void CBrushLibTVCtrl::Create(CWnd * pParent,DWORD idCtrlTree,DWORD idCtrlGrid,DWORD idBnt,const char * name,BOOL bSupportChecked)
{
	_wndGrid.Create(pParent,idCtrlGrid);
	_wndGrid.SetWindowText(fromMBCS(name));

	CRect rc;
	HIDE_CONTROL(pParent,idCtrlTree);
	GET_CONTROL_RECT(pParent,idCtrlTree,rc);
	if(bSupportChecked){
		_wndTree.Create(pParent,rc,idCtrlTree,TVS_CHECKBOXES);
		_wndTree._bSupportCheckBox = TRUE;
	}
	else{
		_wndTree.Create(pParent,rc,idCtrlTree);
		_wndTree._bSupportCheckBox = FALSE;
	}

	//创建source safe 功能Button
	GET_CONTROL_RECT(pParent,idBnt,rc);
	_sscBt.Create(_T(""), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rc, pParent, IDC_BT_SSC);

	SscBtnCallback dlgbt;
	dlgbt.bind(this,&CBrushLibTVCtrl::_OnLoad);
	_sscBt.BindNotifyLoad(dlgbt);
	dlgbt.bind(this,&CBrushLibTVCtrl::_OnSave);
	_sscBt.BindNotifySave(dlgbt);

	_wndGrid._owner = this;
}

BOOL CBrushLibTVCtrl::_OnLoad()
{
	if(_pLib)
		_pLib->ReLoad();
	return TRUE;
}

BOOL CBrushLibTVCtrl::_OnSave()
{
	if(_pLib)
		_pLib->Save();
	return TRUE;
}

void CBrushLibTVCtrl::Bind(IBrushLib * pLib)
{
	if(!pLib){
		_pLib = NULL;
		_ver = 0;
		_wndGrid.BindData(NULL);
		_wndTree.SetNodeTree(NULL);
		_wndTree.EnableWindow(FALSE);
		_wndGrid.EnableWindow(FALSE);
		_sscBt.Bind("",NULL);
	}
	else{
		if(pLib!=_pLib||_ver!=pLib->GetMemVersion()){
			_pLib = pLib;
			_ver = pLib->GetMemVersion();
			BRUID * pIDX = NULL;
			DWORD c = 0;

// 			BRUID brID = GetSelUID();

			_wndTree.UpdateNodeTree(_pLib->GetNodeTree());
// 			extern void RecordTreeCtrlState(CTreeCtrl *ctrl,TreeCtrlState &state,const char *sep);
// 
// 			TreeCtrlState state;
// 			RecordTreeCtrlState(&_wndTree,state,"\\");
// 			if (state.empty())
// 			{
// 				_wndTree.LockPaint();
// 				_wndTree.SetNodeTree();
// 				RestoreTreeCtrlState(&_wndTree,state,"\\");
// 				_wndTree.UnLockPaint();
// 			}
// 			_wndTree.Expand(TVI_ROOT,TVE_EXPAND);
		
// 			if(_pLib->Get(brID)){
// 				SetSelUID(brID);
// 			}
// 			else
// 			{
// 				BRUID * pUID = NULL;
// 				DWORD count = 0;
// 				_pLib->Enum(pUID,count);
// 				if(count>0)
// 					SetSelUID(pUID[0]);
// 			}
		}

		//更新SSC的状态
		_sscBt.Bind(_pLib->GetPath(),_ssc);
		_sscBt.SetFont(_sscBt.GetParent()->GetFont());

		if(_CheckEditable()){
			_wndTree.EnableEdit(TRUE);
			_wndGrid.SetReadOnly(FALSE);
		}
		else{
			_wndTree.EnableEdit(FALSE);
			_wndGrid.SetReadOnly(TRUE);
		}
	}
	
	const IBrush * br = GetSelBrush();
	_wndGrid.BindData(br);
}

BOOL CBrushLibTVCtrl::_CheckEditable()
{
	IFileSystem * pFS = NULL;
	if(_ssc)
		pFS	= _ssc->GetFS();
	
	if(pFS){
		FileAttr atr = pFS->GetFileAttr(_pLib->GetPath());
		return (atr==File_Default);
	}
	
	return TRUE;
}

void CBrushLibTVCtrl::SetSelUID(const BRUID &uid)
{
	if(_pLib){
		NodeTreeRef * ref = _pLib->GetNodeTree();
		CNodeTree * tree = ref->tree;
		if(tree){
			DWORD count = 0;
			NodeHandle * handles = tree->Enum(NodeHandle_Root,NodeType_None,count);
			for(DWORD i = 0;i<count;i++){
				BRUID brID = _pLib->GetBrushID(handles[i]);
				if(brID==uid){
					HTREEITEM hTree = _wndTree.ItemFromNodeHandle(handles[i]);
					if(hTree!=TVI_ROOT){
						_wndTree.SelectItem(hTree);
						_wndTree.EnsureVisible(hTree);
					}
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void CBrushLibTVCtrl::CBrushLibGrid::OnItemChange(CXTPPropertyGridItem *item)
{
	if(_owner->_pLib&&_owner->_CheckEditable()){
		IBrush  * br = GetData();
		_owner->_pLib->Set(br->GetUID(),br);
	}
}

void CBrushLibTVCtrl::CBrushLibGrid::OnEndItemChange(CXTPPropertyGridItem *item)
{
	if(_owner->_pLib&&_owner->_CheckEditable()){
		IBrush * br = GetData();
		if(br)
			_owner->_pLib->Set(br->GetUID(),br);
		_owner->_pLib->Save();
	}
	CLyObjGrid<IBrush>::OnEndItemChange(item);
}

//////////////////////////////////////////////////////////////////////////
const IBrush * CBrushLibTVCtrl::GetSelBrush() 
{
	const IBrush * br = NULL;

	if(_pLib){
		BRUID brID = GetSelUID();
		br = _pLib->Get(brID);
	}

	return br;
}

BRUID CBrushLibTVCtrl::GetSelUID()
{
	BRUID brID = INVALID_BRUID;
	if(_pLib){
		NodeHandle nodeHandle = _wndTree.GetCurSel();
		brID = _pLib->GetBrushID(nodeHandle);
	}
	return brID;
}





