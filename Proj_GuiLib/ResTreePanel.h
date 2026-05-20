#pragma once

#include "GuiLib.h"

#include <map>
#include <string>

#include "ResTree.h"





struct ResData;
class ResDataGroup;
class CResBrowseBtn;
class CRdgHistory;
class IUtilRS;
class CModManager;

class GuiLib_Api CResTreePanel:public CWnd
{
public:
	CResTreePanel()
	{
		_idTimer=0;
	}
	BOOL Create(CWnd *pParent,RECT &rc,UINT id);//Create window
	CResTree *GetTree();

	void SetRootPathAndHistory(const char *path);

	void SetRootPath(const char *path);
	const char *GetRootPath();
protected:
	CResTree _tree;
	CXTPToolBar _toolbar;
	UINT _idTimer;

	std::string _sSel;

	void _LoadHistory();
	void _SaveHistory();

	void _RefreshHistory();
	void _AddToHistory(const char *path);

	std::vector<std::string> _historyPath;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFolderPath();
	afx_msg void OnTimer(UINT_PTR idEvent);
	afx_msg void OnSetHistory(NMHDR* pNMHDR, LRESULT* pRes);
};

