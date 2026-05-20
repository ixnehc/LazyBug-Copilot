#pragma once

#include "GuiLib.h"

#include "ShaderLibTree.h"




class GuiLib_Api CShaderLibPanel:public CWnd
{
public:
	CShaderLibPanel()
	{
		_idTimer=0;
		_global=NULL;
		_ver=0;
	}
	BOOL Create(CWnd *pParent,RECT &rc,UINT id);//Create window

	void SetContent(CShaderLibGlobal *global);
	CShaderLibTree *GetTree()	{		return &_tree;	}

protected:
	UINT _idTimer;
	CShaderLibTree _tree;

	CShaderLibGlobal *_global;

	DWORD _ver;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR idEvent);
};

