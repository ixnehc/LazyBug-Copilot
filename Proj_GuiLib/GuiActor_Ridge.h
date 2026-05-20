
#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

class CGuiAgent_RidgePoints;
class GuiLib_Api CGuiPanel_Ridge :public CGuiPanel
{
public:
	CGuiPanel_Ridge(CWnd * pParent = NULL);
	virtual ~CGuiPanel_Ridge();
	virtual BOOL Create(CWnd *pParent);
	virtual const char *GetName(){ return "ridge";}
	virtual const char *_GetModMgrName()	{		return "world";	}

	virtual void UpdateUI();
	virtual void OnEnterActivity();
	virtual void OnDetachView(CGeView *view,DWORD iLevel);
	void Reset(){}

protected:	
	DECLARE_MESSAGE_MAP() 

	void DoDataExchange(CDataExchange* pDX);
	void OnRidgeCreate();

private:
	CButton _btnStateCreate;
	CGuiAgent_RidgePoints * _agentPoints;
};



