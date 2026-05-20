#pragma once

#include "GuiLib.h"

#include "resource.h"

#include "EditorPanel.h"

#include "WorldSystem/IWorldSystemDefines.h"

#include "GObjGrid.h"

class IAssetClassLib;
class IAssetClass;


class GuiLib_Api CAssetClassPage:public CGObjGrid
{
public:
	CAssetClassPage()
	{
		Zero();
	}
	void Zero()
	{
		_cls=NULL;
		_bReadOnly=FALSE;
	}
	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle = LBS_OWNERDRAWFIXED| LBS_NOINTEGRALHEIGHT);

	void Reset();
	void Bind(IAssetClassLib *classlib,IAssetClass *cls);

	virtual void OnEndItemChange(CXTPPropertyGridItem *item);

protected:
	void _Bind();

	IAssetClass *_cls;
	IAssetClassLib *_classlib;
	BOOL _bReadOnly;

public:
	DECLARE_MESSAGE_MAP()
};


class GuiLib_Api CEditorPanel_Ap:public CEditorPanel
{
public:
	CEditorPanel_Ap(CWnd* pParent = NULL);

	BOOL Create(CWnd *pParent)	{		return CDialog::Create(IDD,pParent);	}

	IAssetClassLib *GetClassLib()	{		return _classlib;	}

	enum { IDD = IDD_EDITPANEL_AP};

	virtual void SetEnv(EditorEnv &env);
	virtual void OnInitAgent();//use DefineEditorAgent(xxx) to define agents

	virtual void OnUpdateUI();

protected:
	CAssetClassPage *_GetPage();
	void _RefreshClass();
	void _Arrange();

	IAssetClassLib *_classlib;
	IAssetClass *_cls;

	//controls
	CAssetClassPage *_page;

public:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
