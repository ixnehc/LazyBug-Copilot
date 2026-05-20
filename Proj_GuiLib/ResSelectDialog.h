#pragma  once

#include "GuiLib.h"
#include ".\resource.h"
#include "RenderSystem/IRenderSystem.h"
#include "ResTree.h"
#include "resdata/ResDataDefines.h"
#include "TreeCtrlBase.h"

class GuiLib_Api CResSelectDialog :public CDialog
{
public:
	CResSelectDialog();
	const char * GetResPath();

	void SetRootPath(const char *pathRoot);
	void SetFilter(ResType filter)	{		_filter=filter;	}
    void SetDefaultResPath(const char *path)    {        _resPath = path;    }
	// dialog template
	enum {IDD = IDD_RESCHOOSEDLG};
protected:
	

	DECLARE_MESSAGE_MAP()	
	afx_msg virtual BOOL OnInitDialog();
	afx_msg void OnChangeFilter();
	afx_msg LRESULT OnMouseSelect(WPARAM wParam,LPARAM lParam);
	
private:
	//static member

	std::string   _rootPath;
	ResType  _filter;
	std::string _resPath;

	std::vector<std::string>_filelist;
	std::vector<std::string>_folderlist;

	TreeCtrlState _treestate;

	CResTree  _showTree;
public:
	afx_msg void OnBnClickedResslectanchor();
};



