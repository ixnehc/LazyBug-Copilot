
#pragma once
#include "GuiLib.h"

#include "RichGrid.h"



class CResAnchorBase;

struct StubArg;
// CStubPopup 对话框
class GuiLib_Api CStubPopup : public CXTPDialog
{
// 构造
public:
	enum EditMask
	{
		Name=1,
		PropClass=2,
		Sem=4,
		Desc=8,
		Connectable=16,
	};

	CStubPopup(CWnd* pParent = NULL);	// 标准构造函数

	BOOL Popup(StubArg *arg);


// 对话框数据
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:

	CRichGrid _grid;
	StubArg *_arg;//编辑的数据

	GSemCode _code;
	std::string _type;
	std::string _nameGVT;


	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
};

