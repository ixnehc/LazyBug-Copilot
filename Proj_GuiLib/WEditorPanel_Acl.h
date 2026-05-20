#pragma once

#include "GuiLib.h"

#include "resource.h"

#include "EditorPanel.h"

#include "WorldSystem/IWorldSystemDefines.h"

#include "NodeTreeCtrl.h"


class GuiLib_Api CAclTree:public CNodeTreeCtrl
{
public:
	CAclTree()
	{
	}

protected:
	virtual UINT _GetImageID();
	virtual DWORD _GetImageIdx(NodeType type);


};


class IAssetClassLib;
class GuiLib_Api CEditorPanel_Acl:public CEditorPanel
{
public:
	CEditorPanel_Acl(CWnd* pParent = NULL);

	BOOL Create(CWnd *pParent)	{		return CDialog::Create(IDD,pParent);	}

	enum { IDD = IDD_EDITPANEL_ACL};

	virtual void SetEnv(EditorEnv &env);
	virtual void OnInitAgent();//use DefineEditorAgent(xxx) to define agents

	virtual void OnUpdateUI();

	virtual BOOL OnEvent(EditorEvent &evnt);


protected:

	CAclTree _tree;

	IAssetClassLib *_classlib;


public:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
