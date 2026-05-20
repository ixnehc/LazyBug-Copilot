#ifndef __ResourceListCtrl_H__
#define __ResourceListCtrl_H__
#include "ResSelectorDefines.h"
#include "RenderSystem/IRenderSystem.h"

#include "NodeTreeCtrl.h"

const int RLCF_DIR			= 1;
const int RLCF_FILE			= 0;

const int RL_ITEM_HEIGHT	= 20;

#define WM_RL_BROWSEDIR		WM_USER + 1000
#define WM_RL_SELCHANGED	WM_USER + 1001

#include <afxtempl.h>

class CResourceListCtrl : public CXTTreeCtrl /*CListBox*/, public CResourceList
{
public:
	CResourceListCtrl(IRenderSystem* rs);
	virtual ~CResourceListCtrl();

public:
	/*!
	\brief
		Set the root directory, op limits in the dir.  
	*/
	virtual void SetRootDir(const char* dir);

	/*!
	\brief
		Set the filter option, 
	*/
	virtual void SetFilter(const char* filter);

	/*!
	\brief
		Return the selected resource relative path.
	*/
	virtual const char* GetSelResource() const;

	//////////////////////////////////////////////////////////////////////////
public:
	void SetBrowseDir(const char* dir,const char *sel="");
	const char* GetBrowseDir() const
	{
		return _browseDir.c_str();
	}

	BOOL IsPassFilter(const char* file) const;

	const char * GetSelResource2();

// Operations
public:
	HTREEITEM AddItem(LPCSTR lpszFileName, DWORD userData);
public:
	// Generated message map functions
protected:

	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
	
	virtual void PreSubclassWindow();

protected:
	void _CreateImageList();

protected:
	IRenderSystem* _pRS;
	std::string _filter;
	std::string _browseDir;
	std::string _selResource;

	CImageList _imageList;
	std::string _rootPath;
	BOOL  _bLocked;
};
#endif