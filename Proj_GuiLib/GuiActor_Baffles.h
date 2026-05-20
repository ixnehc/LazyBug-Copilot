
#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

class CGuiAgent_BafflesPoints;
class GuiLib_Api CGuiPanel_Baffles :public CGuiPanel
{
public:
	CGuiPanel_Baffles(CWnd * pParent = NULL);
	virtual ~CGuiPanel_Baffles();
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
	CGuiAgent_BafflesPoints * _agentPoints;
	CButton _bntStateCreate;
};



