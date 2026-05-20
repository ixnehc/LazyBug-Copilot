
#pragma  once

#include "WorldSystem/ISpt.h"

#include "NodeTreeCtrl.h"

#include "progress/progress.h"

class CBakeTreeCtrl :public CNodeTreeCtrl
{
protected:
	virtual DWORD _GetImageIdx(NodeHandle hNode,SscState state);
	virtual UINT  _GetImageID();
	virtual BOOL _SupportCheckBox(){return TRUE;}
};

class CBakeLibDlg :public CDialog
{
public:
	CBakeLibDlg();
	virtual ~CBakeLibDlg();
	void SetBakeParam(AssetSystemState *ss,IBrushLib *pLib);
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnStartBake();
	afx_msg void OnSelectAll();
	afx_msg void OnDeselectAll();

	void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	void _Messge(const char * msg);
	void _SetCheckState(BOOL bChecked);
	void _EnumSelectModels();

	BOOL OnProcess(const char * desc,int cur,int full);

private:
	IBrushLib * _pLib;
	AssetSystemState *_ss;
	int _iProcess;
	CBakeTreeCtrl _treeCtrlLib;
	std::vector<BRUID> _treeModels;
	HANDLE _hThread;	
	CProgress _progress;
	CProgressCtrl _progressBar;
	std::string _errorMsg;
	DWORD _fTimeStart;
};



