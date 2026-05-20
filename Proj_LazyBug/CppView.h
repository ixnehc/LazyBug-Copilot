#pragma once

#include "ScintillaView.h"

#include <unordered_map>

/////////////////////////////////////////////////////////////////////////////
// CCppView view


class CCppView: public CScintillaView
{
protected:
	CCppView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCppView)

// Attributes
public:

// Operations
public:
	void UpdateSyntaxAssist();
	void UpdateUI();

	void SetExeIndicator(int iLine,BOOL bOnStack);

	int FindFunction(const char *nameFunc);
	void AddFunction(const char *nameFunc);
	int FindVar(const char *nameVar);

	const char *GetCurWord(int pos=-1);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCppView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CCppView();

	void _CullVar(int pos,std::string &s);
	void _CullPrevVar(int pos,std::string &s);
	DWORD _ParseLineHead(int iLine,std::string &head);//返回一行开始有多少个tab,head里返回这行的第一个word
	BOOL _ParseLineTail(int iLine,std::string &tail);//返回某行结束的那个word,将忽略行末的空白

	void _RepairExpect(int iLine,const char *expect);

	virtual void _OnCharAdded(int ch);
	virtual void _OnModified();
	virtual void _OnDwellStart(int pos);
	virtual void _OnDwellEnd(int pos);
	virtual void _OnUpdateUI();


	virtual void _SetDefaultFormat();
//	void _UpdateSyntaxAssist(IProtoLib *lib,IProto *proto,IProtoNode *node);

	std::string _keywords;
	std::string _nodenames;
	std::string _funcnames;

	BOOL _bStdAC;//这个标志表示标准的AutoComplete是否载入了
	std::unordered_map<std::string,std::string>_ac;//auto complete的列表
	void _LoadStdAssist();

	void _UpdatePickHelp();

	BOOL _bLoading;

	// Generated message map functions
protected:
	//{{AFX_MSG(CCppView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	afx_msg void OnFileSave();
	afx_msg void OnUndo();
	afx_msg void OnRedo();
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnCut();
	afx_msg void OnContextMenu( CWnd *, CPoint );

	DECLARE_MESSAGE_MAP()

	friend class CChildView;
};
