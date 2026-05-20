#pragma once

#include "GuiLib.h"

#include "EditorBase.h"


class GuiLib_Api CEditorPanel:public CDialog,public CGuiEditor
{
public:
	CEditorPanel(UINT id,CWnd* pParent ):CDialog(id, pParent)
	{
	}

	//Overidable
	virtual void OnUpdateUI()	{	}

	//overriding
	virtual BOOL Respond(CtrlOp &co);


	virtual void OnOK()	{	}
	virtual void OnCancel(){	}


};


template <class T_EditorPanel>
class CWEA_EditorPanel:public CGuiEditorAgent
{
public:
	T_EditorPanel *GetParent()
	{
		return dynamic_cast<T_EditorPanel *>(GetEditor());
	}
};