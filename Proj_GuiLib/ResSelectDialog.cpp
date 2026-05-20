#include "stdh.h"
#include "ResSelectDialog.h"
#include "WndBase.h"
#include "Log/LogFile.h"
#include "ResAnchor.h"
#include ".\resselectdialog.h"
#include "WMGuiLib.h"

#include "stringparser/stringparser.h"


BEGIN_MESSAGE_MAP(CResSelectDialog,CDialog)
	ON_MESSAGE(GLM_ResTree_DblClick,OnMouseSelect)
	ON_BN_CLICKED(IDC_RESSLECTANCHOR, OnBnClickedResslectanchor)
END_MESSAGE_MAP()

CResSelectDialog::CResSelectDialog()
:CDialog(IDD,NULL)
{
	_filter=Res_None;
}

void CResSelectDialog::SetRootPath(const char *pathRoot)
{
	_rootPath=pathRoot;
	IFileSystem_EnumAllR(g_ssGuiLib.pFS,pathRoot,_filelist,_folderlist);

}

BOOL CResSelectDialog::OnInitDialog()
{
	if(!CDialog::OnInitDialog())
		return FALSE;
	
	CRect rc;
	GET_CONTROL_RECT(this,IDC_RESFILTERTREE,rc);
	_showTree.Create(this,rc,IDC_RESFILTERTREE);
	_showTree.SetOwner(m_hWnd);

	_showTree.EnableEdit(FALSE);

	_showTree.LockPaint();
	_showTree.SetContent(_rootPath.c_str(),_filelist,_folderlist,_filter);

	RestoreTreeCtrlState(&_showTree,_treestate,"\\");

    if (!_resPath.empty())
    {
        HTREEITEM hItem = _showTree.ItemFromPath(_resPath);
        if (hItem != NULL)
        {
			_showTree.CTreeCtrl::EnsureVisible(hItem);
            _showTree.SelectItem(hItem);
        }
    }

	_showTree.UnLockPaint();



	std::string showText = _rootPath+"(点击刷新)";
	SetDlgItemText(IDC_RESSLECTANCHOR, fromMBCS(showText.c_str()));

	_resPath="";

	return TRUE;
}

const char * CResSelectDialog::GetResPath()
{
	return _resPath.c_str();
}

LRESULT CResSelectDialog::OnMouseSelect(WPARAM wParam,LPARAM lParam)
{
	_resPath=(const char *)wParam;

	if (CheckPathContaining(_rootPath.c_str(),_resPath.c_str()))
		_resPath=CutHeadPath(_resPath.c_str(),_rootPath.c_str());
	else
		_resPath="";

	_treestate.clear();
	RecordTreeCtrlState(&_showTree,_treestate,"\\");

	OnOK();

	return TRUE;
}


void CResSelectDialog::OnBnClickedResslectanchor()
{
	RecordTreeCtrlState(&_showTree,_treestate,"\\");

	IFileSystem_EnumAllR(g_ssGuiLib.pFS,_rootPath.c_str(),_filelist,_folderlist);
	_showTree.LockPaint();
	_showTree.SetContent(_rootPath.c_str(),_filelist,_folderlist,_filter);

	RestoreTreeCtrlState(&_showTree,_treestate,"\\");
	_showTree.UnLockPaint();


}

