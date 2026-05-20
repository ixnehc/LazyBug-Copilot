
#pragma once
#include "GuiLib.h"

#include "GObjGrid.h"

#include "WorldSystem/IMapFileArgs.h"

#include "SscBtn.h"

struct MapFileArgs_New
{
	std::string path;
	MapFileArgs arg;
	BEGIN_GOBJ_PURE(MapFileArgs_New,1)
		GELEM_STRING_INIT(path,"");
			GELEM_EDITVAR("Map File Path",GVT_String,GSem_MapFilePath,"map file的路径");
		GELEM_OBJ(MapFileArgs,arg);
			GELEM_EDITOBJ("Args","map file的参数");
	END_GOBJ();

};


class IMapFile;
// CMfArgDlg 对话框
class GuiLib_Api CMfArgDlg : public CDialog
{
// 构造
public:
	CMfArgDlg(CWnd* pParent = NULL);	// 标准构造函数

	void SetMapFile(IMapFile *mf)	{		_mf=mf;	}

	BOOL Create(CWnd *pParent);

	void GetArg(MapFileArgs &arg);
	const char *GetPath();

	BOOL IsNewAndAdd()	{		return _bNewAndAdd;	}

// 对话框数据
	protected:
	virtual void OnOK();

protected:

	IMapFile *_mf;

	CGObjGrid _grid;

	MapFileArgs_New _arg;

	BOOL _bNewAndAdd;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedNewandadd();
};


class GuiLib_Api CMfArgDlg2 : public CDialog
{
	// 构造
public:
	CMfArgDlg2 (CWnd* pParent = NULL);	// 标准构造函数

	void SetMapFile(IMapFile *mf)	{		_mf=mf;	}

	BOOL Create(CWnd *pParent);

	// 对话框数据
protected:

	BOOL _OnSave();
	BOOL _OnLoad();

protected:
	IMapFile *_mf;

	CGObjGrid _grid;
	CSscBtn _btnSsc;	

	MapFileArgs _arg;

	UINT _idTimer;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
};
