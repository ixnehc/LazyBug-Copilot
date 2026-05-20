
#include "stdh.h"

#include "BakeLibDlg.h"

#include <process.h>

#include "RenderSystem/IRenderSystem.h"

#include "resource.h"


//////////////////////////////////////////////////////////////////////////

DWORD CBakeTreeCtrl::_GetImageIdx(NodeHandle hNode,SscState state)
{
	CNodeTree * nodetree =  GetNodeTree()->GetTree();
	if (!nodetree)
		return 0;

	DWORD itype = nodetree->GetType(hNode) - 1;
	return itype;
}



UINT  CBakeTreeCtrl::_GetImageID()	
{		
	return IDB_IMAGE_SPTLIB;	
}
//////////////////////////////////////////////////////////////////////////
CBakeLibDlg::CBakeLibDlg()
:CDialog(IDD_DIALOG_BAKELIB,NULL)
{
	_ss = NULL;
	_pLib = NULL;
	_hThread = NULL;
	_iProcess = 0;
}
CBakeLibDlg::~CBakeLibDlg()
{
}
void CBakeLibDlg::SetBakeParam(AssetSystemState *ss,IBrushLib *pLib)
{
	_pLib = pLib;
	_ss = ss;
}

BEGIN_MESSAGE_MAP(CBakeLibDlg,CDialog)
	ON_COMMAND(IDC_BUTTON_BAKESTART,OnStartBake)
	ON_COMMAND(IDC_BUTTON_SELECTALL,OnSelectAll)
	ON_COMMAND(IDC_BUTTON_DESELECTALL,OnDeselectAll)
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CBakeLibDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX,IDC_TREE_TREELIB,_treeCtrlLib);
	DDX_Control(pDX,IDC_PROGRESS_BAKELIB,_progressBar);
}

void CBakeLibDlg::OnSelectAll()
{
	_SetCheckState(TRUE);
}

void CBakeLibDlg::OnDeselectAll()
{
	_SetCheckState(FALSE);
}

void CBakeLibDlg::_SetCheckState(BOOL bChecked)
{
	CNodeTree * nodeTree = _treeCtrlLib.GetNodeTree()->GetTree();
	if(nodeTree){
		NodeHandle * handles = NULL;
		DWORD c = 0;
		handles = nodeTree->Enum((void *)(uintptr_t)(0xffffffff),NodeType_None,c); //node speed tree
		for(DWORD i = 0;i<c;i++){
			HTREEITEM item = _treeCtrlLib.ItemFromNodeHandle(handles[i]);
			_treeCtrlLib.SetChecked(item,bChecked);
		}
	}
}

void CBakeLibDlg::OnStartBake()
{
	_treeModels.clear();

	_EnumSelectModels();

	if(!_treeModels.empty()){

		CWnd * pWnd = GetDlgItem(IDC_BUTTON_BAKESTART);
		if(pWnd)
			pWnd->EnableWindow(FALSE);
		
		_fTimeStart = GetTickCount();

		_iProcess = 0;
		for(int i = 0;i<_treeModels.size();i++){
			_pLib->Bake(_ss,_treeModels[i],&_progress);
			_iProcess++;
		}
		
		//将消息堆栈的消息消耗光
		MSG message;
		while(PeekMessage(&message,m_hWnd,0,0,PM_REMOVE)){
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if(pWnd)
			pWnd->EnableWindow(TRUE);
	}

	CDialog::OnOK();
}

void CBakeLibDlg:: _EnumSelectModels()
{
	if(!_pLib){
		_Messge("Speed tree libarary lost!");
		return;
	}

	CNodeTree * nodeTree = _treeCtrlLib.GetNodeTree()->GetTree();
	if(nodeTree){
		NodeHandle * handles = NULL;
		DWORD c = 0;
		handles = nodeTree->Enum((void *)(uintptr_t)(0xffffffff),NodeType(2),c); //node speed tree
		for(DWORD i = 0;i<c;i++){
			HTREEITEM item = _treeCtrlLib.ItemFromNodeHandle(handles[i]);
			if(_treeCtrlLib.IsChecked(item)){
				BRUID uid = _pLib->GetBrushID(handles[i]);
				const IBrush * br = _pLib->Get(uid);
				if(br&&_pLib->ObtainRes(br))
					_treeModels.push_back(uid);
			}	
		}
	}
}

BOOL CBakeLibDlg::OnProcess(const char * desc,int cur,int full)
{
	if(full<=0)
		return TRUE;

	float vProcess = float(cur)/float(full);
	float nTree = float(_treeModels.size());
	float ratio = (float(_iProcess)+vProcess)/nTree;
	
	int leftTime = 0;
	int minutes = 0;
	int sec = 0;
	if(ratio>0){
		DWORD tTime = GetTickCount();
		tTime = (tTime - _fTimeStart)/1000;
		float totalTime = tTime/ratio;
		leftTime = int(totalTime - tTime); //剩余时间
		minutes = leftTime/60;
		sec = leftTime%60;
	}

	CString strMessage;
	if(ratio>0){
		if(minutes>0)	
			strMessage.Format(_T("当前:%s.\r\n完成: %.1f%% \r\n剩下:%d 分钟。"), desc, ratio * 100.0f, minutes);
		else
			strMessage.Format(_T("当前:%s.\r\n完成: %.1f%% \r\n剩下:%d 秒。"),desc,ratio*100.0f,sec);
	}
	else
		strMessage.Format(_T("当前:%s.\r\n完成: %.1f%% "),desc,ratio*100.0f);

	SetDlgItemText(IDC_STATIC_BAKEINFO,strMessage);
	
	_progressBar.SetPos(int(ratio*100));

	return TRUE;
}

void CBakeLibDlg::_Messge(const char * msg)
{
	SetDlgItemText(IDC_STATIC_BAKEINFO, fromMBCS(msg));
}

BOOL CBakeLibDlg::OnInitDialog()
{
	if(FALSE==CDialog::OnInitDialog())
		return FALSE;
	
	NodeTreeRef * refNode = _pLib->GetNodeTree();
	_treeCtrlLib.SetNodeTree(refNode);
	
	PrgHandler_SetProgess fun;
	fun.bind(this,&CBakeLibDlg::OnProcess);
	_progress.SetHandler(fun);

	_progressBar.SetRange(0,100);
	_progressBar.SetStep(1);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////



