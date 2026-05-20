
#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

class CGuiAgent_BaffleMove;
class GuiLib_Api CGuiPanel_Baffle :public CGuiPanel
{
public:
	CGuiPanel_Baffle(CWnd * pParent = NULL);
	virtual ~CGuiPanel_Baffle();
	virtual BOOL Create(CWnd *pParent);
	virtual const char *GetName(){ return "baffle";}
	virtual const char *_GetModMgrName()	{		return "world";	}

	virtual void UpdateUI();
	virtual void OnEnterActivity();
	virtual void OnDetachView(CGeView *view,DWORD iLevel);
	void Reset(){}

protected:	
	DECLARE_MESSAGE_MAP() 
	void DoDataExchange(CDataExchange* pDX);
	void OnBaffleCreate();

private:
	BOOL  _bOnCreate; //
	CGuiAgent_BaffleMove * _agentMove;
};



