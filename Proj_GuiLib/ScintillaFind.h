
#pragma once
#include "GuiLib.h"


//this is copied from scintilla.h
// #define SCFIND_WHOLEWORD 2
// #define SCFIND_MATCHCASE 4



struct FindArg
{
	FindArg()
	{
		bMatchCase=TRUE;
		bMatchWord=TRUE;
	}
	BOOL bMatchCase;
	BOOL bMatchWord;
};

struct FindCmd
{
	FindCmd()
	{
		op=None;
	}
	enum Op
	{
		None,
		Find,
		FindInProto,
		FindInAll,
		Replace,
		ReplaceAll,
	};
	Op op;
	std::string str;
	std::string str2;
	FindArg arg;
};


class GuiLib_Api CScintillaFind : public CXTPDialog
{
// 构造
public:
	CScintillaFind(CWnd* pParent = NULL);	// 标准构造函数

	BOOL Create(CWnd *pParent);

	void Activate()	
	{		
		_bShow=TRUE;	
		_bStartShow=TRUE;  
	}

	void SetText(const char *str);
	void Update(CWnd *owner);


// 对话框数据
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual void OnOK();
	virtual void OnCancel();


// 实现
protected:
	void _CollectCmd(FindCmd &cmd);

	void _UpdateArg();
	FindArg _arg;

	BOOL _bShow;
	BOOL _bStartShow;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnMatchCase();
	afx_msg void OnMatchWord();
	afx_msg void OnFindInProto();
	afx_msg void OnFindInAll();
	DECLARE_MESSAGE_MAP()
public:
};
